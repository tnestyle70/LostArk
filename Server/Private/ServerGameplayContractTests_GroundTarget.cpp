#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests_PlayerSkillFixtures.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "Network/PacketReader.h"
#include "ServerNavigation.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_GroundTarget(TESTS& tests, CGameplayCatalog& catalog)
{

	CServerNavigation groundTargetNavigation;
	const bool groundTargetNavigationLoaded =
		groundTargetNavigation.Load("LV_LUT_HEARTRB_ED");
	tests.Require_GroundTarget(groundTargetNavigationLoaded,
		"Load authoritative navigation for ground-target skill contracts");
	for (const QUICK_SKILL_CONTRACT& contract : QUICK_SKILLS)
	{
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(contract.skillId);
		const std::string_view inputSlot = contract.inputSlot;
		const bool expectedCounterCapability =
			"Q" == inputSlot || "W" == inputSlot ||
			"E" == inputSlot || "R" == inputSlot || 34580u == contract.skillId;
		tests.Require(
			nullptr != skill &&
			skill->eCharacterClass == contract.characterClass &&
			skill->strInputSlot == contract.inputSlot,
			"Resolve playable skill binding");
		tests.Require(
			nullptr != skill && skill->iCounterPower ==
				(expectedCounterCapability ? 1u : 0u),
			"Resolve the exact Q/W/E/R and authored guard counter capability");
		tests.Require(
			nullptr != skill &&
			(skill->strDamageProfileId.empty() ?
				0u == catalog.Find_DamageRatePercent(skill->strDamageProfileId) :
				0u != catalog.Find_DamageRatePercent(skill->strDamageProfileId)),
			"Resolve playable skill damage policy");

		SERVER_PLAYER quickPlayer{};
		quickPlayer.eCharacterClass = contract.characterClass;
		quickPlayer.eStance = nullptr != skill ?
			skill->eRequiredStance : PLAYER_STANCE_ID::NONE;
		quickPlayer.iCurrentHp = 1;
		quickPlayer.iMaximumHp = 1;
		/* Official CostMp runs 206..938 at the reference level, so the test pool
		matches the published class pool rather than the old 100. */
		quickPlayer.iCurrentResource = 1000;
		quickPlayer.iMaximumResource = 1000;
		/* Same idea for identityCost (Artist's moon/sun orbs): a class without
		a gauge finds nullptr and stays at 0, which is correct there too. */
		const PLAYER_RUNTIME_PROFILE* quickIdentityProfile =
			catalog.Find_Player(contract.characterClass);
		quickPlayer.iMaximumIdentity = nullptr != quickIdentityProfile ?
			quickIdentityProfile->iMaximumIdentity : 0u;
		quickPlayer.iCurrentIdentity = quickPlayer.iMaximumIdentity;
		C2S_USE_SKILL quickCommand{};
		quickCommand.iClientSequence = 1;
		quickCommand.iSkillId = contract.skillId;
		const bool isGroundTargetSkill = nullptr != skill &&
			SKILL_TARGET_INTENT_KIND::GROUND_POINT == skill->eTargetIntent;
		if (isGroundTargetSkill)
		{
			quickPlayer.fPositionX = 147.75f;
			quickPlayer.fPositionZ = -117.25f;
			quickCommand.eTargetIntent =
				SKILL_TARGET_INTENT_KIND::GROUND_POINT;
			quickCommand.fAimX = 156.25f;
			quickCommand.fAimZ = -122.25f;
		}
		else
		{
			quickCommand.fAimX = 1.f;
			quickCommand.fAimZ = 0.f;
		}
		CPlayerSkillSystem quickSkillSystem;
		tests.Require(
			quickSkillSystem.Try_Start(
				quickPlayer,
				quickCommand,
				catalog,
				1,
				isGroundTargetSkill ? &groundTargetNavigation : nullptr),
			"Approve playable skill command");
	}
	{
		constexpr SKILL_ID DIMENSIONMASTER_T = 2050500u;
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(DIMENSIONMASTER_T);
		SERVER_NAV_POINT approvedTarget{};
		const bool targetSampled = groundTargetNavigation.Sample_Position(
			156.25f, -122.25f, approvedTarget);
		const bool exactTargetingDefinition = nullptr != skill &&
			SKILL_TARGET_INTENT_KIND::GROUND_POINT == skill->eTargetIntent &&
			std::fabs(skill->fTargetMaximumRange - 11.f) < 0.0001f &&
			skill->requiresWalkableTarget && 938u == skill->iResourceCost &&
			4367u == skill->iActionDurationMs &&
			2858u == skill->iHitTimeMs;
		tests.Require_GroundTarget(exactTargetingDefinition && targetSampled,
			"Resolve DimensionMaster T as exact 11m walkable ground-target skill");

		auto makePlayer = []()
		{
			SERVER_PLAYER player{};
			player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			player.fPositionX = 147.75f;
			player.fPositionY = 0.f;
			player.fPositionZ = -117.25f;
			return player;
		};
		const auto gameplayStateUnchanged = [](
			const SERVER_PLAYER& before,
			const SERVER_PLAYER& after)
		{
			return before.iLastSkillSequence == after.iLastSkillSequence &&
				before.iCurrentResource == after.iCurrentResource &&
				before.eAction == after.eAction &&
				before.iCurrentSkillId == after.iCurrentSkillId &&
				before.CooldownEndTickBySkillId ==
					after.CooldownEndTickBySkillId &&
				before.hasSkillTarget == after.hasSkillTarget;
		};

		CPlayerSkillSystem skills;
		SERVER_PLAYER rejected = makePlayer();
		C2S_USE_SKILL command{};
		command.iClientSequence = 1u;
		command.iSkillId = DIMENSIONMASTER_T;
		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		const SERVER_PLAYER beforeWrongIntent = rejected;
		const bool wrongIntentRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeWrongIntent, rejected);

		command.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
		command.fAimX = rejected.fPositionX + 11.01f;
		command.fAimZ = rejected.fPositionZ;
		const SERVER_PLAYER beforeOutOfRange = rejected;
		const bool outOfRangeRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeOutOfRange, rejected);

		command.fAimX = std::numeric_limits<float>::quiet_NaN();
		const SERVER_PLAYER beforeNonFinite = rejected;
		const bool nonFiniteRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeNonFinite, rejected);

		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		command.iSkillId = 0xfefefefeu;
		const SERVER_PLAYER beforeUnknown = rejected;
		const bool unknownRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeUnknown, rejected);
		tests.Require_GroundTarget(
			wrongIntentRejected && outOfRangeRejected &&
			nonFiniteRejected && unknownRejected,
			"Reject wrong intent, out-of-range, non-finite and unknown T without mutation");

		float lastWalkableX = 152.f;
		float firstBlockedX = 0.f;
		SERVER_NAV_POINT navProbe{};
		bool foundNearbyBlockedPoint = groundTargetNavigation.Sample_Position(
			lastWalkableX, -137.f, navProbe);
		const float probeStep = groundTargetNavigation.Get_CellSize() * 0.25f;
		if (foundNearbyBlockedPoint)
		{
			foundNearbyBlockedPoint = false;
			const float probeLimitX = lastWalkableX + 60.f;
			for (float x = lastWalkableX + probeStep;
				x <= probeLimitX; x += probeStep)
			{
				if (groundTargetNavigation.Sample_Position(x, -137.f, navProbe))
				{
					lastWalkableX = x;
					continue;
				}
				firstBlockedX = x;
				foundNearbyBlockedPoint = true;
				break;
			}
		}
		SERVER_PLAYER navRejected = makePlayer();
		navRejected.fPositionX = lastWalkableX;
		navRejected.fPositionZ = -137.f;
		command.iSkillId = DIMENSIONMASTER_T;
		command.iClientSequence = 1u;
		command.fAimX = firstBlockedX;
		command.fAimZ = -137.f;
		const SERVER_PLAYER beforeNavReject = navRejected;
		const bool navInvalidRejected = foundNearbyBlockedPoint &&
			!skills.Try_Start(navRejected, command, catalog, 10u,
				&groundTargetNavigation) &&
			gameplayStateUnchanged(beforeNavReject, navRejected);
		tests.Require_GroundTarget(navInvalidRejected,
			"Reject a within-range but non-walkable DimensionMaster T target transactionally");

		SERVER_PLAYER approved = makePlayer();
		command.iClientSequence = 1u;
		command.iSkillId = DIMENSIONMASTER_T;
		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		const bool started = skills.Try_Start(
			approved, command, catalog, 20u, &groundTargetNavigation);
		const std::uint32_t resourceAfterStart = approved.iCurrentResource;
		const auto cooldownsAfterStart = approved.CooldownEndTickBySkillId;
		const bool approvedTargetPreserved = started &&
			PLAYER_ACTION_STATE::SKILL == approved.eAction &&
			approved.hasSkillTarget &&
			std::fabs(approved.fSkillTargetX - approvedTarget.x) < 0.0001f &&
			std::fabs(approved.fSkillTargetY - approvedTarget.y) < 0.0001f &&
			std::fabs(approved.fSkillTargetZ - approvedTarget.z) < 0.0001f &&
			1000u - skill->iResourceCost == resourceAfterStart &&
			cooldownsAfterStart.contains(DIMENSIONMASTER_T);
		const bool duplicateRejected = !skills.Try_Start(
			approved, command, catalog, 21u, &groundTargetNavigation) &&
			resourceAfterStart == approved.iCurrentResource &&
			cooldownsAfterStart == approved.CooldownEndTickBySkillId &&
			1u == approved.iLastSkillSequence;
		tests.Require_GroundTarget(approvedTargetPreserved && duplicateRejected,
			"Commit approved T target/cost once and reject duplicate sequence without mutation");

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto snapshotRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
		CGameRoom& snapshotRoom = *snapshotRoomStorage;
		constexpr SESSION_ID SNAPSHOT_SESSION = 91001u;
		constexpr PLAYER_ID SNAPSHOT_PLAYER = 91001u;
		auto snapshotSession = std::make_shared<CClientSession>(
			SNAPSHOT_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		snapshotSession->m_isSendRunning.store(true);
		SERVER_PLAYER snapshotPlayer = approved;
		snapshotPlayer.iSessionId = SNAPSHOT_SESSION;
		snapshotPlayer.iPlayerId = SNAPSHOT_PLAYER;
		snapshotPlayer.iNetEntityId = 91001u;
		snapshotRoom.m_iServerTick = 20u;
		snapshotRoom.m_Sessions.emplace(SNAPSHOT_SESSION, snapshotSession);
		snapshotRoom.m_PlayerIdBySessionId.emplace(
			SNAPSHOT_SESSION, SNAPSHOT_PLAYER);
		snapshotRoom.m_Players.emplace(SNAPSHOT_PLAYER, snapshotPlayer);
		snapshotRoom.Broadcast_WorldSnapshot();
		S2C_WORLD_SNAPSHOT decodedSnapshot{};
		bool snapshotTargetPreserved = false;
		if (1u == snapshotSession->m_OutboundFrames.size())
		{
			const std::vector<std::uint8_t>& frame =
				snapshotSession->m_OutboundFrames.front().Bytes;
			PACKET_HEADER header{};
			if (Read_Packet_Header(frame, header) &&
				PACKET_TYPE::S2C_WORLD_SNAPSHOT == header.ePacketType &&
				header.iTotalSize == frame.size())
			{
				CPacketReader reader{ std::span<const std::uint8_t>(
					frame.data() + PACKET_HEADER_BYTES,
					frame.size() - PACKET_HEADER_BYTES) };
				if (Read_Message(reader, decodedSnapshot) &&
					0u == reader.Get_RemainingSize())
				{
					const auto replicated = std::find_if(
						decodedSnapshot.Players.begin(),
						decodedSnapshot.Players.end(),
						[](const PLAYER_SNAPSHOT& player)
						{ return 91001u == player.iNetEntityId; });
					snapshotTargetPreserved =
						decodedSnapshot.Players.end() != replicated &&
						replicated->hasSkillTarget &&
						std::fabs(replicated->fSkillTargetX - approvedTarget.x) < 0.0001f &&
						std::fabs(replicated->fSkillTargetY - approvedTarget.y) < 0.0001f &&
						std::fabs(replicated->fSkillTargetZ - approvedTarget.z) < 0.0001f &&
						decodedSnapshot.ActiveGameplayRevision ==
							snapshotRoom.m_GameplayCatalog.Get_ActiveRevision() &&
						decodedSnapshot.RequiredPinnedGameplayRevisions.empty() &&
						std::all_of(
							decodedSnapshot.Entities.begin(),
							decodedSnapshot.Entities.end(),
							[&decodedSnapshot](const WORLD_ENTITY_SNAPSHOT& entity)
							{
								return entity.PinnedDefinitionRevision ==
									decodedSnapshot.ActiveGameplayRevision;
							});
				}
			}
		}
		snapshotSession->Request_Close();
		tests.Require_GroundTarget(snapshotTargetPreserved,
			"Replicate the approved T target XYZ through the canonical world snapshot");

		SERVER_WORLD_ENTITY atTarget{};
		atTarget.iNetEntityId = 92001u;
		atTarget.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		atTarget.eAction = SERVER_ENTITY_ACTION::IDLE;
		atTarget.iCurrentHp = 100000u;
		atTarget.iMaximumHp = 100000u;
		atTarget.fPositionX = approvedTarget.x;
		atTarget.fPositionY = approvedTarget.y;
		atTarget.fPositionZ = approvedTarget.z;
		atTarget.fCollisionRadius = 0.25f;
		SERVER_WORLD_ENTITY atCaster = atTarget;
		atCaster.iNetEntityId = 92002u;
		atCaster.fPositionX = approved.fPositionX;
		atCaster.fPositionY = approved.fPositionY;
		atCaster.fPositionZ = approved.fPositionZ;
		std::vector<SERVER_WORLD_ENTITY> targets{ atTarget, atCaster };
		std::vector<DAMAGE_EVENT> damageEvents;
		skills.Update(approved, targets, catalog, &groundTargetNavigation,
			nullptr, 2.859f, 21u, damageEvents);
		const bool damagedAtApprovedRoot = 1u == damageEvents.size() &&
			92001u == damageEvents.front().iTargetNetEntityId &&
			targets[0].iCurrentHp < targets[0].iMaximumHp &&
			targets[1].iCurrentHp == targets[1].iMaximumHp;
		tests.Require_GroundTarget(damagedAtApprovedRoot,
			"Resolve DimensionMaster T damage at approved target root, not caster root");

		skills.Update(approved, targets, catalog, &groundTargetNavigation,
			nullptr, 2.f, 22u, damageEvents);
		tests.Require_GroundTarget(
			PLAYER_ACTION_STATE::NONE == approved.eAction &&
			!approved.hasSkillTarget &&
			0.f == approved.fSkillTargetX && 0.f == approved.fSkillTargetY &&
			0.f == approved.fSkillTargetZ,
			"Clear approved T target atomically when the action ends");

		SERVER_PLAYER pending = makePlayer();
		pending.eAction = PLAYER_ACTION_STATE::SKILL;
		pending.iCurrentSkillId = 2050010u;
		pending.iLastSkillSequence = 1u;
		command.iClientSequence = 2u;
		command.iSkillId = DIMENSIONMASTER_T;
		command.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		const bool pendingStaged = skills.Try_StagePendingSkill(
			pending, command, catalog, &groundTargetNavigation);
		pending.eAction = PLAYER_ACTION_STATE::NONE;
		pending.iCurrentSkillId = INVALID_SKILL_ID;
		pending.fPositionX = approvedTarget.x - 20.f;
		pending.fPositionZ = approvedTarget.z;
		pending.PendingCommand.Clear();
		const std::uint32_t pendingResource = pending.iCurrentResource;
		const auto pendingCooldowns = pending.CooldownEndTickBySkillId;
		const bool pendingRevalidated = !skills.Try_StartPending(
			pending, command, catalog, 30u, &groundTargetNavigation) &&
			pendingResource == pending.iCurrentResource &&
			pendingCooldowns == pending.CooldownEndTickBySkillId &&
			!pending.hasSkillTarget &&
			PLAYER_ACTION_STATE::NONE == pending.eAction;
		tests.Require_GroundTarget(pendingStaged && pendingRevalidated,
			"Revalidate a buffered T target at actual start and reject stale range without cost");

		SERVER_PLAYER interrupted = makePlayer();
		command.iClientSequence = 1u;
		const bool interruptedStarted = skills.Try_Start(
			interrupted, command, catalog, 40u, &groundTargetNavigation);
		interrupted.iCurrentHp = 0u;
		std::vector<SERVER_WORLD_ENTITY> noTargets;
		std::vector<DAMAGE_EVENT> noDamageEvents;
		skills.Update(interrupted, noTargets, catalog, &groundTargetNavigation,
			nullptr, 0.f, 41u, noDamageEvents);
		const bool deathCleared = interruptedStarted &&
			PLAYER_ACTION_STATE::DEAD == interrupted.eAction &&
			INVALID_SKILL_ID == interrupted.iCurrentSkillId &&
			!interrupted.hasSkillTarget &&
			0.f == interrupted.fSkillTargetX &&
			0.f == interrupted.fSkillTargetY &&
			0.f == interrupted.fSkillTargetZ;

		SERVER_PLAYER knockedDown = makePlayer();
		command.iClientSequence = 1u;
		const bool knockdownStarted = skills.Try_Start(
			knockedDown, command, catalog, 50u, &groundTargetNavigation);
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			knockedDown, knockedDown.fPositionX - 1.f,
			knockedDown.fPositionZ, 1.f, 100u, true, 1000u, 51u);
		const bool knockdownCleared = knockdownStarted &&
			PLAYER_ACTION_STATE::KNOCKDOWN == knockedDown.eAction &&
			INVALID_SKILL_ID == knockedDown.iCurrentSkillId &&
			!knockedDown.hasSkillTarget &&
			0.f == knockedDown.fSkillTargetX &&
			0.f == knockedDown.fSkillTargetY &&
			0.f == knockedDown.fSkillTargetZ;
		tests.Require_GroundTarget(deathCleared && knockdownCleared,
			"Clear approved T target on death and knockdown interruption");
	}
}

