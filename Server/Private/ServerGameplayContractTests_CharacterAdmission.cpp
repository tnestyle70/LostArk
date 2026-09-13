#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "Gameplay/WorldCollisionContract.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerNavigation.h"
#include "ServerApp.h"
#include "ServerTriggerSystem.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_CharacterAdmission(TESTS& tests, CGameplayCatalog& catalog)
{

	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 700u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.fCollisionRadius = 0.6f;
		monster.fAttackRange = 1.f;
		monster.fEngageDistance = 8.f;
		monster.fMoveSpeed = 2.f;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER protectedPlayer{};
		protectedPlayer.iPlayerId = 701u;
		protectedPlayer.iNetEntityId = 702u;
		protectedPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		protectedPlayer.iCurrentHp = 100u;
		protectedPlayer.iMaximumHp = 100u;
		protectedPlayer.fPositionX = 1.f;
		protectedPlayer.isCombatReady = false;
		players.emplace(protectedPlayer.iPlayerId, protectedPlayer);
		std::vector<DAMAGE_EVENT> damageEvents;
		/* This case owns one loose monster rather than a room's entity list, so
		it has no neighbour to separate from. */
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 1u, damageEvents);
		const bool ignoredProtectedPlayer =
			INVALID_NET_ENTITY_ID == monster.iTargetEntityId &&
			SERVER_ENTITY_ACTION::IDLE == monster.eAction;
		players.begin()->second.isCombatReady = true;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 2u, damageEvents);
		tests.Require(
			navigationLoaded && ignoredProtectedPlayer &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction &&
			players.begin()->second.iNetEntityId == monster.iTargetEntityId,
			"Ignore protected players and acquire the same player after combat admission");
	}
	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 750u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.eAction = SERVER_ENTITY_ACTION::CHASE;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.iTargetEntityId = 752u;
		monster.iNextPathReplanTick = 100u;
		monster.fYawDegrees = 1e20f;
		monster.fCollisionRadius = 0.1f;
		monster.fAttackRange = 0.01f;
		monster.fEngageDistance = 10.f;
		monster.fTargetReleaseDistance = 14.f;
		monster.fMoveSpeed = 2.f;
		monster.fTurnSpeedDegreesPerSecond = 180.f;
		monster.fMoveAcceleration = 4.f;
		monster.fMoveDeceleration = 6.f;
		monster.fArrivalSlowRadius = 1.f;
		monster.MovePath.push_back({ 2.f, 0.f, 0.f });

		SERVER_PLAYER player{};
		player.iPlayerId = 751u;
		player.iNetEntityId = 752u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		player.fPositionX = 5.f;
		player.isCombatReady = true;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		players.emplace(player.iPlayerId, player);
		std::vector<DAMAGE_EVENT> damageEvents;
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 10u, damageEvents);
		tests.Require(
			navigationLoaded && std::isfinite(monster.fYawDegrees) &&
			std::fabs(monster.fYawDegrees) <= 180.f,
			"Normalize an extreme monster yaw in bounded time");
	}
	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 800u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.fCollisionRadius = 0.1f;
		monster.fAttackRange = 0.01f;
		monster.fEngageDistance = 10.f;
		monster.fTargetReleaseDistance = 14.f;
		monster.fMoveSpeed = 2.f;
		monster.fTurnSpeedDegreesPerSecond = 180.f;
		monster.fMoveAcceleration = 4.f;
		monster.fMoveDeceleration = 6.f;
		monster.fArrivalSlowRadius = 1.f;
		monster.iPatternTelegraphMs = 1000u;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		for (std::uint32_t index = 0u; index < 2u; ++index)
		{
			SERVER_PLAYER player{};
			player.iPlayerId = 810u + index;
			player.iNetEntityId = 820u + index;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.iCurrentHp = 100u;
			player.iMaximumHp = 100u;
			player.fPositionX = 5.f + static_cast<float>(index);
			player.isCombatReady = true;
			players.emplace(player.iPlayerId, player);
		}
		std::vector<DAMAGE_EVENT> damageEvents;
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 10u, damageEvents);
		const NET_ENTITY_ID firstTarget = monster.iTargetEntityId;
		players.find(811u)->second.fPositionX = 4.f;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 11u, damageEvents);
		const bool retainedTarget = firstTarget == monster.iTargetEntityId;
		players.find(810u)->second.fPositionX = 20.f;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 12u, damageEvents);
		const bool releasedAndRetargeted =
			821u == monster.iTargetEntityId;
		monster.eAction = SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		monster.fActionElapsedSeconds = 0.f;
		players.find(810u)->second.fPositionX = 0.5f;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 13u, damageEvents);
		tests.Require(
			navigationLoaded && 820u == firstTarget && retainedTarget &&
			releasedAndRetargeted && 821u == monster.iTargetEntityId,
			"Retain a valid monster target, release it at hysteresis range, and lock the replacement through windup");
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		CGameRoom& room = *roomStorage;
		tests.Require(room.Is_Ready(),
			"Initialize Character Select room for class changes");
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			room.Find_AvailablePlayerSpawn();
		tests.Require(nullptr != spawn,
			"Resolve Character Select class-change respawn placement");
		if (room.Is_Ready() && nullptr != spawn)
		{
			SERVER_PLAYER player{};
			player.iSessionId = 11u;
			player.iPlayerId = 12u;
			player.iNetEntityId = 112u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "ClassSwitch";
			player.strSpawnPlacementId = spawn->strPlacementId;
			player.fPositionX = 17.f;
			player.fPositionY = 3.f;
			player.fPositionZ = -9.f;
			player.fYawDegrees = 33.f;
			player.iCurrentHp = 10u;
			player.iMaximumHp = 100u;
			player.iCurrentResource = 2u;
			player.iMaximumResource = 10u;
			player.iCurrentMadness = 250u;
			player.iMaximumMadness = 500u;
			player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
			player.eAction = PLAYER_ACTION_STATE::SKILL;
			player.iCurrentSkillId = 34120u;
			player.iActionStartTick = 9u;
			player.iLastMoveSequence = 7u;
			player.iLastSkillSequence = 8u;
			player.hasMoveGoal = true;
			player.CooldownEndTickBySkillId.emplace(34120u, 100u);
			C2S_MOVE pendingBeforeClassChange{};
			pendingBeforeClassChange.iClientSequence = 8u;
			pendingBeforeClassChange.fGoalX = 20.f;
			pendingBeforeClassChange.fGoalZ = -10.f;
			player.PendingCommand.Set_Move(pendingBeforeClassChange);

			C2S_CHANGE_CHARACTER_CLASS request{};
			request.iClientSequence = 1u;
			request.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass &&
				17.f == player.fPositionX && -9.f == player.fPositionZ &&
				12u == player.iPlayerId && 112u == player.iNetEntityId &&
				7u == player.iLastMoveSequence &&
				8u == player.iLastSkillSequence &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				INVALID_SKILL_ID == player.iCurrentSkillId &&
				PLAYER_PENDING_COMMAND_KIND::NONE == player.PendingCommand.eKind &&
				!player.hasMoveGoal && player.CooldownEndTickBySkillId.empty() &&
				player.iCurrentHp == player.iMaximumHp &&
				player.iCurrentResource == player.iMaximumResource &&
				0u == player.iCurrentMadness &&
				SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM == player.iMaximumMadness &&
				PLAYER_MADNESS_FORM::NORMAL == player.eMadnessForm,
				"Change class during action, preserve identity/position/sequences, and reset state");

			C2S_USE_SKILL oldClassSkill{};
			oldClassSkill.iClientSequence = 9u;
			oldClassSkill.iSkillId = 34120u;
			oldClassSkill.fAimX = 1.f;
			oldClassSkill.fAimZ = 0.f;
			C2S_USE_SKILL newClassSkill = oldClassSkill;
			newClassSkill.iSkillId = 31200u;
			tests.Require(
				!room.m_PlayerSkillSystem.Try_Start(
					player, oldClassSkill, room.m_GameplayCatalog, 10u) &&
				room.m_PlayerSkillSystem.Try_Start(
					player, newClassSkill, room.m_GameplayCatalog, 10u) &&
				31200u == player.iCurrentSkillId &&
				9u == player.iLastSkillSequence,
				"Reject old-class skill and approve new-class skill after class change");

			const SERVER_PLAYER accepted = player;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE ==
					room.Apply_CharacterClassChange(player, request) &&
				accepted.eCharacterClass == player.eCharacterClass &&
				accepted.iCurrentHp == player.iCurrentHp,
				"Reject stale class change without mutating player");

			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.fPositionX = 999.f;
			player.fPositionY = 999.f;
			player.fPositionZ = 999.f;
			request.iClientSequence = 2u;
			request.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			SERVER_NAV_POINT projected{};
			const bool projectedSpawn = room.m_ServerNavigation.Project_Point(
				spawn->fPositionX, spawn->fPositionZ, projected);
			tests.Require(projectedSpawn &&
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass &&
				projected.x == player.fPositionX &&
				projected.y == player.fPositionY &&
				projected.z == player.fPositionZ &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				0u != player.iCurrentHp,
				"Change dead player class and respawn at projected original spawn");

			/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
			auto bernRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::BERN);
			CGameRoom& bernRoom = *bernRoomStorage;
			const SERVER_PLAYER beforeWrongWorld = player;
			request.iClientSequence = 3u;
			request.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
			tests.Require(bernRoom.Is_Ready() &&
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD ==
					bernRoom.Apply_CharacterClassChange(player, request) &&
				beforeWrongWorld.eCharacterClass == player.eCharacterClass &&
				beforeWrongWorld.iCurrentHp == player.iCurrentHp,
				"Reject class change outside Character Select without mutation");
		}
	}
	{
		auto bernEntryRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::BERN);
		CGameRoom& bernEntryRoom = *bernEntryRoomStorage;
		const auto& bernEntryPlacements =
			bernEntryRoom.m_WorldBootstrap.Get_Placements();
		const std::size_t expectedNpcCount = static_cast<std::size_t>(
			std::count_if(
				bernEntryPlacements.begin(), bernEntryPlacements.end(),
				[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					return placement.isEnabled &&
						WORLD_BOOTSTRAP_KIND::NPC == placement.eKind;
				}));
		std::size_t serializedNpcCount = 0u;
		bool allWorldSpawnsSerializable = bernEntryRoom.Is_Ready();
		SERVER_WORLD_ENTITY invalidNpc{};
		bool foundNpc = false;
		for (const SERVER_WORLD_ENTITY& entity : bernEntryRoom.m_WorldEntities)
		{
			std::vector<std::uint8_t> payload;
			S2C_WORLD_ENTITY_SPAWNED decoded{};
			if (!CGameRoom::Build_WorldEntitySpawnedPayload(entity, payload))
			{
				allWorldSpawnsSerializable = false;
				continue;
			}
			CPacketReader reader{ payload };
			if (!Read_Message(reader, decoded) ||
				0u != reader.Get_RemainingSize() ||
				decoded.iNetEntityId != entity.iNetEntityId ||
				decoded.strPlacementId != entity.strPlacementId ||
				decoded.PinnedDefinitionRevision !=
					entity.PinnedDefinitionRevision)
			{
				allWorldSpawnsSerializable = false;
				continue;
			}
			if (WORLD_BOOTSTRAP_KIND::NPC == entity.eKind)
			{
				++serializedNpcCount;
				allWorldSpawnsSerializable = allWorldSpawnsSerializable &&
					WORLD_ENTITY_KIND::NPC == decoded.eKind &&
					0.f == entity.fCollisionRadius &&
					0.f == decoded.fCollisionRadius;
				if (!foundNpc)
				{
					invalidNpc = entity;
					foundNpc = true;
				}
			}
		}
		std::vector<std::uint8_t> unchangedPayload{ 0x7fu };
		if (foundNpc)
		{
			invalidNpc.fCollisionRadius =
				LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X;
			allWorldSpawnsSerializable = allWorldSpawnsSerializable &&
				!CGameRoom::Build_WorldEntitySpawnedPayload(
					invalidNpc, unchangedPayload) &&
				1u == unchangedPayload.size() && 0x7fu == unchangedPayload.front();
		}
		tests.Require(
			allWorldSpawnsSerializable && foundNpc &&
			0u != expectedNpcCount && expectedNpcCount == serializedNpcCount,
			"Preflight every Bern world spawn and keep town NPC wire radius zero");

		constexpr SESSION_ID BERN_ENTRY_SESSION = 73001u;
		auto bernEntrySession = std::make_shared<CClientSession>(
			BERN_ENTRY_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		bernEntrySession->m_isSendRunning.store(true);
		bernEntryRoom.m_Sessions.insert_or_assign(
			BERN_ENTRY_SESSION, bernEntrySession);
		C2S_ENTER_WORLD bernEnter{};
		bernEnter.eWorldId = WORLD_ID::BERN;
		bernEnter.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		bernEnter.strNickName = "BernEntryContract";
		const bool joinedBern = bernEntryRoom.Join(
			BERN_ENTRY_SESSION, bernEnter);
		std::size_t worldSpawnFrameCount = 0u;
		bool sawLocalPlayerSpawn = false;
		for (const CClientSession::OUTBOUND_FRAME& frame :
			bernEntrySession->m_OutboundFrames)
		{
			if (PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED == frame.ePacketType)
				++worldSpawnFrameCount;
			else if (PACKET_TYPE::S2C_PLAYER_SPAWNED == frame.ePacketType)
				sawLocalPlayerSpawn = true;
		}
		const bool completeBernEntryStream = joinedBern && sawLocalPlayerSpawn &&
			worldSpawnFrameCount == bernEntryRoom.m_WorldEntities.size();
		bernEntrySession->Request_Close();
		tests.Require(completeBernEntryStream,
			"Join Bern with every world spawn and the local player spawn queued");
	}
	{
		struct PARTY_FIXTURE final
		{
			std::unique_ptr<CServerApp> App = std::make_unique<CServerApp>();
			std::shared_ptr<CGameRoom> Source = std::make_shared<CGameRoom>(WORLD_ID::BERN);
			std::shared_ptr<CGameRoom> Target = std::make_shared<CGameRoom>(WORLD_ID::VALTAN_ARENA);
			std::vector<std::shared_ptr<CClientSession>> Sessions;
			SERVER_WORLD_TRANSFER_REQUEST Request;
			bool Ready = false;
		};
		const auto clearOutbound = [](CClientSession& session)
		{
			session.m_OutboundFrames.clear();
			session.m_iQueuedOutboundBytes = 0u;
			session.m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
			session.m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
		};
		{
			auto first = std::make_shared<CClientSession>(95991u, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			auto second = std::make_shared<CClientSession>(95992u, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			first->m_isSendRunning.store(true);
			second->m_isSendRunning.store(true);
			// Reproduce the close interval after terminal diagnostic publication,
			// before Request_Close has changed running flags or cleared its queue.
			second->Record_TerminalDiagnostic(SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED,
				0, "contract terminal admission race");
			S2C_PARTY_TRANSFER_RESULT notice{ 81u, WORLD_ID::VALTAN_ARENA,
				PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE };
			CPacketWriter writer;
			const bool encoded = Write_Message(writer, notice);
			const PACKET_FRAME frame{ PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT, writer.Get_Buffer() };
			CClientSession::RELIABLE_BATCH_TRANSACTION admission;
			std::string status;
			tests.Require(encoded && !admission.Prepare({ { first, { frame } }, { second, { frame } } }, status) &&
				second->m_isSendRunning.load() && first->m_OutboundFrames.empty() &&
				second->m_OutboundFrames.empty() && !status.empty(),
				"Reject terminal-in-progress admission and release every staged participant unchanged");
		}
		const auto makeParty = [&clearOutbound](const std::size_t count, const bool formParty)
		{
			auto fixture = std::make_unique<PARTY_FIXTURE>();
			fixture->Ready = fixture->Source->Is_Ready() && fixture->Target->Is_Ready();
			fixture->App->m_SharedGameRooms.emplace(WORLD_ID::BERN, fixture->Source);
			fixture->App->m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, fixture->Target);
			const CHARACTER_CLASS_ID classes[] = { CHARACTER_CLASS_ID::LANCE_MASTER,
				CHARACTER_CLASS_ID::ARTIST, CHARACTER_CLASS_ID::WARLORD,
				CHARACTER_CLASS_ID::DIMENSIONMASTER };
			for (std::size_t index = 0; index < count && fixture->Ready; ++index)
			{
				const SESSION_ID id = 96001u + index;
				auto session = std::make_shared<CClientSession>(id, INVALID_SOCKET,
					CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
				session->m_isSendRunning.store(true);
				fixture->Source->Handle_Register(session);
				C2S_ENTER_WORLD enter{};
				enter.eWorldId = WORLD_ID::BERN;
				enter.eCharacterClass = classes[index];
				enter.strNickName = "PartyContract" + std::to_string(index);
				fixture->Ready = fixture->Source->Join(id, enter);
				fixture->Sessions.push_back(session);
				fixture->App->m_Sessions.emplace(id, session);
				CServerApp::SESSION_GAMEPLAY_BINDING binding{};
				binding.eWorldId = WORLD_ID::BERN;
				binding.pSimulation = fixture->Source;
				fixture->App->m_GameplayBindingBySessionId.emplace(id, binding);
				for (const auto& active : fixture->Sessions) clearOutbound(*active);
			}
			if (!fixture->Ready) return fixture;
			const auto& leader = fixture->Source->m_Players.at(fixture->Sessions.front()->Get_PlayerId());
			fixture->Request.iSessionId = leader.iSessionId;
			fixture->Request.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
			fixture->Request.eCharacterClass = leader.eCharacterClass;
			fixture->Request.strNickName = leader.strNickName;
			fixture->Request.iPartyRequestSequence = 81u;
			for (const auto& session : fixture->Sessions)
				fixture->Request.PartyBatchSessionIds.push_back(session->Get_SessionId());
			if (formParty)
			{
				for (std::size_t index = 1; index < count; ++index)
				{
					C2S_PARTY_INVITE invite{};
					invite.iRequestSequence = static_cast<std::uint32_t>(index);
					invite.iTargetNetEntityId = fixture->Source->m_Players.at(
						fixture->Sessions[index]->Get_PlayerId()).iNetEntityId;
					fixture->Source->Handle_PartyInvite(leader.iSessionId, invite);
					C2S_PARTY_INVITE_RESPOND respond{};
					respond.iRequestSequence = static_cast<std::uint32_t>(index);
					respond.iFromNetEntityId = leader.iNetEntityId;
					respond.bAccepted = true;
					fixture->Source->Handle_PartyInviteRespond(
						fixture->Sessions[index]->Get_SessionId(), respond);
				}
				fixture->Ready = 1u == fixture->Source->m_PartyMembersByPartyId.size() &&
					count == fixture->Source->m_PartyMembersByPartyId.begin()->second.size();
			}
			for (const auto& active : fixture->Sessions) clearOutbound(*active);
			return fixture;
		};
		for (const std::size_t count : { 2u, 4u })
		{
			auto fixture = makeParty(count, true);
			tests.Require(fixture->Ready, "Create a real two/four-player invited party fixture");
			if (!fixture->Ready) continue;
			const auto guide = std::find_if(fixture->Source->m_WorldEntities.begin(),
				fixture->Source->m_WorldEntities.end(), [](const SERVER_WORLD_ENTITY& entity)
				{ return "npc.bern.beda.guide" == entity.strPlacementId; });
			bool leaderBatch = guide != fixture->Source->m_WorldEntities.end();
			if (leaderBatch)
			{
				for (const auto& session : fixture->Sessions)
				{
					auto& player = fixture->Source->m_Players.at(session->Get_PlayerId());
					player.fPositionX = guide->fPositionX;
					player.fPositionZ = guide->fPositionZ;
				}
				C2S_CONFIRM_NPC_ENTRY confirm{};
				confirm.iRequestSequence = 81u;
				confirm.strNpcPlacementId = guide->strPlacementId;
				fixture->Source->Handle_ConfirmNpcEntry(
					fixture->Sessions[1]->Get_SessionId(), confirm);
				bool hasFailureNotice = false;
				for (const auto& frame : fixture->Sessions[1]->m_OutboundFrames)
				{
					if (PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT != frame.ePacketType) continue;
					CPacketReader reader{ std::span<const std::uint8_t>{ frame.Bytes }.subspan(PACKET_HEADER_BYTES) };
					S2C_PARTY_TRANSFER_RESULT result{};
					hasFailureNotice = Read_Message(reader, result) &&
						PARTY_TRANSFER_RESULT::REJECTED_NOT_LEADER == result.eResult;
				}
				tests.Require(fixture->Source->m_PendingWorldTransfers.empty() && hasFailureNotice,
					"Reject a non-leader NPC entry without splitting or silently ignoring the party");
				for (const auto& session : fixture->Sessions) clearOutbound(*session);
				fixture->Source->Handle_ConfirmNpcEntry(
					fixture->Sessions.front()->Get_SessionId(), confirm);
				leaderBatch = 1u == fixture->Source->m_PendingWorldTransfers.size() &&
					fixture->Source->Try_DequeueWorldTransfer(fixture->Request) &&
					count == fixture->Request.PartyBatchSessionIds.size();
			}
			CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure{};
			const bool committed = leaderBatch && fixture->App->Transfer_SessionWorld(
				fixture->Source, fixture->Request, failure);
			bool exactRoster = committed && fixture->Source->m_Players.empty() &&
				fixture->Source->m_PartyMembersByPartyId.empty() &&
				count == fixture->Target->m_Players.size() &&
				1u == fixture->Target->m_PartyMembersByPartyId.size();
			for (const auto& session : fixture->Sessions)
			{
				std::size_t accepted = 0u;
				S2C_PARTY_ROSTER roster{};
				for (const auto& frame : session->m_OutboundFrames)
				{
					if (PACKET_TYPE::S2C_ENTER_ACCEPTED == frame.ePacketType) ++accepted;
					if (PACKET_TYPE::S2C_PARTY_ROSTER != frame.ePacketType) continue;
					CPacketReader reader{ std::span<const std::uint8_t>{ frame.Bytes }.subspan(PACKET_HEADER_BYTES) };
					exactRoster = Read_Message(reader, roster) && exactRoster;
				}
				exactRoster = exactRoster && 1u == accepted && count == roster.Members.size() &&
					WORLD_ID::VALTAN_ARENA == fixture->App->m_GameplayBindingBySessionId.at(
						session->Get_SessionId()).eWorldId;
				for (std::size_t index = 0; index < roster.Members.size(); ++index)
					exactRoster = exactRoster && roster.Members[index].strNickname ==
						"PartyContract" + std::to_string(index);
			}
			tests.Require(exactRoster,
				"Atomically transfer the full party and preserve leader-first roster for every member");
		}
		{
			// 파티 레이드 입장 전원 수락 투표 (G2)
			const auto findGuide = [](CGameRoom& room)
			{
				return std::find_if(room.m_WorldEntities.begin(),
					room.m_WorldEntities.end(), [](const SERVER_WORLD_ENTITY& e)
					{ return "npc.bern.beda.guide" == e.strPlacementId; });
			};
			const auto placeAtGuide = [](PARTY_FIXTURE& fixture, float x, float z)
			{
				for (const auto& s : fixture.Sessions)
				{
					auto& p = fixture.Source->m_Players.at(s->Get_PlayerId());
					p.fPositionX = x;
					p.fPositionZ = z;
				}
			};
			// 4인 전원 수락 -> Valtan batch 전송 stage
			{
				auto fixture = makeParty(4u, true);
				tests.Require(fixture->Ready, "Create raid-entry-vote party fixture");
				const auto guide = findGuide(*fixture->Source);
				if (fixture->Ready && guide != fixture->Source->m_WorldEntities.end())
				{
					placeAtGuide(*fixture, guide->fPositionX, guide->fPositionZ);
					for (const auto& s : fixture->Sessions) clearOutbound(*s);
					C2S_RAID_ENTRY_PROPOSE propose{};
					propose.iRequestSequence = 81u;
					propose.strNpcPlacementId = guide->strPlacementId;
					propose.eTarget = RAID_ENTRY_TARGET::VALTAN;
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions.front()->Get_SessionId(), propose);
					bool allPrompted = true;
					for (const auto& s : fixture->Sessions)
					{
						bool got = false;
						for (const auto& f : s->m_OutboundFrames)
							if (PACKET_TYPE::S2C_RAID_ENTRY_PROMPT == f.ePacketType) got = true;
						allPrompted = allPrompted && got;
					}
					tests.Require(allPrompted &&
						1u == fixture->Source->m_RaidEntryProposals.size() &&
						fixture->Source->m_PendingWorldTransfers.empty(),
						"Leader propose prompts every member and stages no transfer yet");
					const std::uint32_t proposalId =
						fixture->Source->m_RaidEntryProposals.front().iProposalId;
					for (const auto& s : fixture->Sessions)
					{
						C2S_RAID_ENTRY_RESPOND respond{};
						respond.iRequestSequence = 1u;
						respond.iProposalId = proposalId;
						respond.bAccepted = true;
						fixture->Source->Handle_RaidEntryRespond(
							s->Get_SessionId(), respond);
					}
					tests.Require(fixture->Source->m_RaidEntryProposals.empty() &&
						1u == fixture->Source->m_PendingWorldTransfers.size() &&
						WORLD_ID::VALTAN_ARENA ==
							fixture->Source->m_PendingWorldTransfers.front().eTargetWorldId &&
						4u == fixture->Source->m_PendingWorldTransfers.front()
							.PartyBatchSessionIds.size(),
						"All members accepting stages exactly one Valtan batch transfer");
				}
			}
			// 한 명 거절 -> 전송 없음, 투표 종료
			{
				auto fixture = makeParty(4u, true);
				const auto guide = findGuide(*fixture->Source);
				if (fixture->Ready && guide != fixture->Source->m_WorldEntities.end())
				{
					placeAtGuide(*fixture, guide->fPositionX, guide->fPositionZ);
					C2S_RAID_ENTRY_PROPOSE propose{};
					propose.iRequestSequence = 82u;
					propose.strNpcPlacementId = guide->strPlacementId;
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions.front()->Get_SessionId(), propose);
					const std::uint32_t proposalId =
						fixture->Source->m_RaidEntryProposals.front().iProposalId;
					for (std::size_t i = 0; i < 3u; ++i)
					{
						C2S_RAID_ENTRY_RESPOND r{};
						r.iRequestSequence = 1u; r.iProposalId = proposalId;
						r.bAccepted = true;
						fixture->Source->Handle_RaidEntryRespond(
							fixture->Sessions[i]->Get_SessionId(), r);
					}
					C2S_RAID_ENTRY_RESPOND decline{};
					decline.iRequestSequence = 1u; decline.iProposalId = proposalId;
					decline.bAccepted = false;
					fixture->Source->Handle_RaidEntryRespond(
						fixture->Sessions[3]->Get_SessionId(), decline);
					tests.Require(fixture->Source->m_RaidEntryProposals.empty() &&
						fixture->Source->m_PendingWorldTransfers.empty(),
						"One decline closes the vote and stages no transfer");
				}
			}
			// 비리더 발의 -> 무시(투표 안 열림)
			{
				auto fixture = makeParty(4u, true);
				const auto guide = findGuide(*fixture->Source);
				if (fixture->Ready && guide != fixture->Source->m_WorldEntities.end())
				{
					placeAtGuide(*fixture, guide->fPositionX, guide->fPositionZ);
					C2S_RAID_ENTRY_PROPOSE propose{};
					propose.iRequestSequence = 83u;
					propose.strNpcPlacementId = guide->strPlacementId;
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions[1]->Get_SessionId(), propose);
					tests.Require(fixture->Source->m_RaidEntryProposals.empty(),
						"Non-leader propose is ignored without opening a vote");
				}
			}
			// 쿠크세이튼 타겟 -> KAKULSAYDON_ARENA 전송
			{
				auto fixture = makeParty(2u, true);
				const auto guide = findGuide(*fixture->Source);
				if (fixture->Ready && guide != fixture->Source->m_WorldEntities.end())
				{
					placeAtGuide(*fixture, guide->fPositionX, guide->fPositionZ);
					C2S_RAID_ENTRY_PROPOSE propose{};
					propose.iRequestSequence = 84u;
					propose.strNpcPlacementId = guide->strPlacementId;
					propose.eTarget = RAID_ENTRY_TARGET::KAKULSAYDON;
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions.front()->Get_SessionId(), propose);
					const std::uint32_t proposalId =
						fixture->Source->m_RaidEntryProposals.front().iProposalId;
					for (const auto& s : fixture->Sessions)
					{
						C2S_RAID_ENTRY_RESPOND r{};
						r.iRequestSequence = 1u; r.iProposalId = proposalId;
						r.bAccepted = true;
						fixture->Source->Handle_RaidEntryRespond(s->Get_SessionId(), r);
					}
					tests.Require(1u == fixture->Source->m_PendingWorldTransfers.size() &&
						WORLD_ID::KAKULSAYDON_ARENA ==
							fixture->Source->m_PendingWorldTransfers.front().eTargetWorldId,
						"Kukusaton target stages a KakulSaydon batch transfer");
				}
			}
			// 타임아웃 -> Expire가 TIMEOUT으로 닫고 전송 없음
			{
				auto fixture = makeParty(2u, true);
				const auto guide = findGuide(*fixture->Source);
				if (fixture->Ready && guide != fixture->Source->m_WorldEntities.end())
				{
					placeAtGuide(*fixture, guide->fPositionX, guide->fPositionZ);
					C2S_RAID_ENTRY_PROPOSE propose{};
					propose.iRequestSequence = 85u;
					propose.strNpcPlacementId = guide->strPlacementId;
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions.front()->Get_SessionId(), propose);
					tests.Require(1u == fixture->Source->m_RaidEntryProposals.size(),
						"Vote opens before timeout");
					fixture->Source->m_iServerTick =
						fixture->Source->m_RaidEntryProposals.front().iDeadlineTick + 1u;
					fixture->Source->Expire_RaidEntryProposals();
					tests.Require(fixture->Source->m_RaidEntryProposals.empty() &&
						fixture->Source->m_PendingWorldTransfers.empty(),
						"Deadline expiry closes the vote with no transfer");
				}
			}
			// voter 이탈 -> Cancel이 CANCELLED로 닫음
			{
				auto fixture = makeParty(2u, true);
				const auto guide = findGuide(*fixture->Source);
				if (fixture->Ready && guide != fixture->Source->m_WorldEntities.end())
				{
					placeAtGuide(*fixture, guide->fPositionX, guide->fPositionZ);
					C2S_RAID_ENTRY_PROPOSE propose{};
					propose.iRequestSequence = 86u;
					propose.strNpcPlacementId = guide->strPlacementId;
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions.front()->Get_SessionId(), propose);
					fixture->Source->Cancel_RaidEntryProposalsInvolving(
						fixture->Sessions[1]->Get_PlayerId());
					tests.Require(fixture->Source->m_RaidEntryProposals.empty() &&
						fixture->Source->m_PendingWorldTransfers.empty(),
						"A voter leaving cancels the vote with no transfer");
				}
			}
			// 중복 발의(열린 투표 존재) -> 무시
			{
				auto fixture = makeParty(2u, true);
				const auto guide = findGuide(*fixture->Source);
				if (fixture->Ready && guide != fixture->Source->m_WorldEntities.end())
				{
					placeAtGuide(*fixture, guide->fPositionX, guide->fPositionZ);
					C2S_RAID_ENTRY_PROPOSE propose{};
					propose.iRequestSequence = 87u;
					propose.strNpcPlacementId = guide->strPlacementId;
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions.front()->Get_SessionId(), propose);
					fixture->Source->Handle_RaidEntryPropose(
						fixture->Sessions.front()->Get_SessionId(), propose);
					tests.Require(1u == fixture->Source->m_RaidEntryProposals.size(),
						"A second propose while a vote is open is ignored");
				}
			}
		}
		{
			auto fixture = makeParty(2u, true);
			tests.Require(fixture->Ready, "Create party transfer rollback fixture");
			if (fixture->Ready)
			{
				for (auto& [id, player] : fixture->Source->m_Players)
				{
					(void)id;
					player.iCurrentHp = 37u;
					player.fPositionX += 0.25f;
				}
				const auto sourcePlayers = fixture->Source->m_Players;
				const auto sourceParties = fixture->Source->m_PartyMembersByPartyId;
				const auto originalCatalog = fixture->Target->m_GameplayCatalog;
				const auto originalWorldEntities = fixture->Target->m_WorldEntities;
				auto& placements = const_cast<std::vector<WORLD_BOOTSTRAP_PLACEMENT>&>(
					fixture->Target->m_WorldBootstrap.Get_Placements());
				std::vector<WORLD_BOOTSTRAP_PLACEMENT*> spawns;
				for (auto& placement : placements)
					if (placement.isEnabled && WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind)
						spawns.push_back(&placement);
				const char* names[] = { "Reject full target without source departure",
					"Reject missing target profile without source departure",
					"Rollback a second-member navigation failure without source departure",
					"Reject a terminal party member without moving the leader",
					"Rollback a second-member reliable queue failure without publishing acceptance",
					"Reject malformed initial world payload before moving any party member" };
				for (std::size_t scenario = 0; scenario < 6u; ++scenario)
				{
					if (spawns.size() < 3u) { tests.Require(false, names[scenario]); continue; }
					const float secondSpawnX = spawns[1]->fPositionX;
					if (0u == scenario)
					{
						for (std::size_t index = 0; index < 3u; ++index)
						{
							SERVER_PLAYER occupied{};
							occupied.iPlayerId = static_cast<PLAYER_ID>(98000u + index);
							occupied.strSpawnPlacementId = spawns[index]->strPlacementId;
							fixture->Target->m_Players.emplace(occupied.iPlayerId, occupied);
						}
					}
					else if (1u == scenario) fixture->Target->m_GameplayCatalog = CGameplayCatalogGenerations{};
					else if (2u == scenario) spawns[1]->fPositionX = std::numeric_limits<float>::quiet_NaN();
					else if (3u == scenario) fixture->Sessions[1]->Request_Close();
					else if (4u == scenario)
					{
						const std::array<std::uint8_t, 1> payload{ 1u };
						for (std::size_t index = 0; index < CClientSession::MAX_OUTBOUND_FRAME_COUNT; ++index)
							(void)fixture->Sessions[1]->Send_Frame(PACKET_TYPE::S2C_CHAT, payload);
					}
					else if (5u == scenario)
					{
						// Valtan can start with no enabled actors. Always insert the
						// malformed payload instead of silently skipping fault injection.
						SERVER_WORLD_ENTITY malformed{};
						malformed.eKind = WORLD_BOOTSTRAP_KIND::NPC;
						malformed.strArchetypeId = "npc.contract.invalid";
						malformed.strPlacementId = "contract.party.malformed";
						malformed.iNetEntityId = INVALID_NET_ENTITY_ID;
						fixture->Target->m_WorldEntities.push_back(std::move(malformed));
					}
					const std::size_t targetCount = fixture->Target->m_Players.size();
					const auto targetNextId = fixture->Target->m_iNextPlayerId;
					CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure{};
					bool preserved = !fixture->App->Transfer_SessionWorld(fixture->Source,
						fixture->Request, failure) && !failure.strContext.empty() &&
						fixture->Source->m_Players.size() == sourcePlayers.size() &&
						fixture->Source->m_PartyMembersByPartyId == sourceParties &&
						fixture->Target->m_Players.size() == targetCount &&
						fixture->Target->m_iNextPlayerId == targetNextId;
					for (std::size_t index = 0; index < fixture->Sessions.size(); ++index)
					{
						const auto& session = fixture->Sessions[index];
						const auto found = fixture->Source->m_Players.find(session->Get_PlayerId());
						preserved = preserved && found != fixture->Source->m_Players.end() &&
							WORLD_ID::BERN == fixture->App->m_GameplayBindingBySessionId.at(session->Get_SessionId()).eWorldId &&
							fixture->Source == fixture->App->m_GameplayBindingBySessionId.at(session->Get_SessionId()).pSimulation;
						if (found != fixture->Source->m_Players.end())
						{
							const auto& before = sourcePlayers.at(found->first);
							preserved = preserved && before.iCurrentHp == found->second.iCurrentHp &&
								before.fPositionX == found->second.fPositionX && before.fPositionZ == found->second.fPositionZ;
						}
						for (const auto& frame : session->m_OutboundFrames)
							preserved = preserved && PACKET_TYPE::S2C_ENTER_ACCEPTED != frame.ePacketType;
						if (3u != scenario || 1u != index) preserved = preserved && !session->Is_Closing();
					}
					tests.Require(preserved, names[scenario]);
					fixture->Target->m_Players.clear();
					fixture->Target->m_GameplayCatalog = originalCatalog;
					spawns[1]->fPositionX = secondSpawnX;
					fixture->Target->m_WorldEntities = originalWorldEntities;
					fixture->Sessions[1]->m_CloseDiagnostic = {};
					fixture->Sessions[1]->m_isSendRunning.store(true);
					for (const auto& session : fixture->Sessions) clearOutbound(*session);
				}
				const std::array<std::uint8_t, 1> payload{ 1u };
				for (std::size_t index = 0; index < CClientSession::MAX_OUTBOUND_FRAME_COUNT; ++index)
					(void)fixture->Sessions[0]->Send_Frame(PACKET_TYPE::S2C_CHAT, payload);
				fixture->Source->Notify_PartyTransferFailure(fixture->Sessions[0]->Get_SessionId(),
					81u, WORLD_ID::VALTAN_ARENA, PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY);
				const bool pendingNotice = !fixture->Sessions[0]->Is_Closing() &&
					1u == fixture->Source->m_PendingPartyTransferResults.size();
				clearOutbound(*fixture->Sessions[0]);
				fixture->Source->Flush_PartyTransferResults();
				tests.Require(pendingNotice && fixture->Source->m_PendingPartyTransferResults.empty() &&
					1u == fixture->Sessions[0]->m_OutboundFrames.size() &&
					PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT == fixture->Sessions[0]->m_OutboundFrames.front().ePacketType,
					"Delay a busy-queue failure notice without disconnecting the preserved source party");
			}
		}
		{
			auto fixture = makeParty(3u, false);
			tests.Require(fixture->Ready, "Create replaced-invitation identity fixture");
			if (fixture->Ready)
			{
				const auto firstId = fixture->Sessions[0]->Get_PlayerId();
				const auto secondId = fixture->Sessions[1]->Get_PlayerId();
				const auto targetId = fixture->Sessions[2]->Get_PlayerId();
				C2S_PARTY_INVITE invite{};
				invite.iRequestSequence = 1u;
				invite.iTargetNetEntityId = fixture->Source->m_Players.at(targetId).iNetEntityId;
				fixture->Source->Handle_PartyInvite(fixture->Sessions[0]->Get_SessionId(), invite);
				fixture->Source->Handle_PartyInvite(fixture->Sessions[1]->Get_SessionId(), invite);
				C2S_PARTY_INVITE_RESPOND respond{};
				respond.iRequestSequence = 2u;
				respond.iFromNetEntityId = fixture->Source->m_Players.at(firstId).iNetEntityId;
				bool preserved = true;
				for (const bool accepted : { false, true })
				{
					respond.bAccepted = accepted;
					fixture->Source->Handle_PartyInviteRespond(fixture->Sessions[2]->Get_SessionId(), respond);
					const auto pending = fixture->Source->m_PendingPartyInviteByTargetPlayerId.find(targetId);
					preserved = preserved && pending != fixture->Source->m_PendingPartyInviteByTargetPlayerId.end() &&
						pending->second == secondId && fixture->Source->m_PartyMembersByPartyId.empty();
				}
				respond.iFromNetEntityId = fixture->Source->m_Players.at(secondId).iNetEntityId;
				fixture->Source->Handle_PartyInviteRespond(fixture->Sessions[2]->Get_SessionId(), respond);
				tests.Require(preserved && fixture->Source->m_PendingPartyInviteByTargetPlayerId.empty() &&
					1u == fixture->Source->m_PartyMembersByPartyId.size(),
					"Stale accept and decline preserve a replacement invite until its exact inviter is answered");
			}
		}
	}
}

