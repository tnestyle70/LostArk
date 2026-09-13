#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "Gameplay/CombatCollisionContract.h"
#include "GameRoom.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_RoomIngress(TESTS& tests)
{

	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
		CGameRoom& room = *roomStorage;
		bool admittedBestEffort = room.Is_Ready();
		for (std::size_t index = 0u;
			index < CGameRoom::MAX_BEST_EFFORT_COMMAND_COUNT; ++index)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::MOVE;
			command.iSessionId = static_cast<SESSION_ID>(index + 1u);
			command.Move.iClientSequence = 1u;
			const bool accepted = room.Enqueue(std::move(command));
			admittedBestEffort = admittedBestEffort && accepted;
			if (!accepted)
				break;
		}
		ROOM_COMMAND droppedMove{};
		droppedMove.eType = ROOM_COMMAND_TYPE::MOVE;
		droppedMove.iSessionId = 5000u;
		droppedMove.Move.iClientSequence = 1u;
		const ROOM_COMMAND_ENQUEUE_RESULT bestEffortDropResult =
			room.Enqueue_Detailed(std::move(droppedMove));
		const bool bestEffortDropWasNonFatal =
			ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT ==
				bestEffortDropResult;

		bool admittedReliableReserve = true;
		while (room.m_InboundCommands.size() <
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
			command.iSessionId = static_cast<SESSION_ID>(
				room.m_InboundCommands.size() + 10000u);
			command.UseSkill.iClientSequence = 1u;
			command.UseSkill.iSkillId = 34010u;
			const bool accepted = room.Enqueue(std::move(command));
			admittedReliableReserve =
				admittedReliableReserve && accepted;
			if (!accepted)
				break;
		}
		ROOM_COMMAND rejectedReliable{};
		rejectedReliable.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		rejectedReliable.iSessionId = 20000u;
		rejectedReliable.UseSkill.iClientSequence = 1u;
		rejectedReliable.UseSkill.iSkillId = 34010u;
		const ROOM_COMMAND_ENQUEUE_RESULT reliableRejectResult =
			room.Enqueue_Detailed(std::move(rejectedReliable));
		const bool reliableRejectedWithoutMutation =
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY ==
				reliableRejectResult &&
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT ==
				room.m_InboundCommands.size();

		SERVER_PLAYER cleanupPlayer{};
		cleanupPlayer.iSessionId = 40000u;
		cleanupPlayer.iPlayerId = 40000u;
		cleanupPlayer.iNetEntityId = 40000u;
		cleanupPlayer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		cleanupPlayer.iCurrentHp = 100u;
		cleanupPlayer.iMaximumHp = 100u;
		room.m_Players.emplace(cleanupPlayer.iPlayerId, cleanupPlayer);
		room.m_PlayerIdBySessionId.emplace(
			cleanupPlayer.iSessionId, cleanupPlayer.iPlayerId);
		room.m_PlayerIdByEntityId.emplace(
			cleanupPlayer.iNetEntityId, cleanupPlayer.iPlayerId);

		ROOM_COMMAND cleanup{};
		cleanup.eType = ROOM_COMMAND_TYPE::LEAVE;
		cleanup.iSessionId = cleanupPlayer.iSessionId;
		cleanup.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool cleanupAcceptedAtReliableHardCap =
			room.Enqueue(std::move(cleanup));
		ROOM_COMMAND duplicateCleanup{};
		duplicateCleanup.eType = ROOM_COMMAND_TYPE::LEAVE;
		duplicateCleanup.iSessionId = cleanupPlayer.iSessionId;
		duplicateCleanup.eLeaveReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		const bool duplicateCleanupDeduplicated =
			ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP ==
			room.Enqueue_Detailed(std::move(duplicateCleanup));
		const bool cleanupWasSeparated =
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT ==
				room.m_InboundCommands.size() &&
			1u == room.m_CleanupCommands.size() &&
			1u == room.m_QueuedCleanupSessionIds.size();
		room.Tick(1.f / 30.f);
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			room.Get_PerformanceMetrics();
		const bool cleanupDrainedBeforeBoundedIngress =
			!room.m_Players.contains(cleanupPlayer.iPlayerId) &&
			!room.m_PlayerIdBySessionId.contains(cleanupPlayer.iSessionId) &&
			!room.m_PlayerIdByEntityId.contains(cleanupPlayer.iNetEntityId) &&
			room.m_CleanupCommands.empty() &&
			room.m_QueuedCleanupSessionIds.empty() &&
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT -
				CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK ==
				room.m_InboundCommands.size();
		tests.Require(
			admittedBestEffort && bestEffortDropWasNonFatal &&
			admittedReliableReserve && reliableRejectedWithoutMutation &&
			cleanupAcceptedAtReliableHardCap &&
			duplicateCleanupDeduplicated && cleanupWasSeparated &&
			cleanupDrainedBeforeBoundedIngress &&
			1u == metrics.iDroppedBestEffortCommandCount &&
			1u == metrics.iRejectedReliableCommandCount &&
			0u == metrics.iRejectedCleanupCommandCount &&
			1u == metrics.iDeduplicatedCleanupCommandCount &&
			1u == metrics.iCleanupIngressHighWatermark &&
			1u == metrics.iLastCleanupIngressDepth &&
			1u == metrics.iLastDrainedCleanupCommandCount &&
			0u == metrics.iLastRemainingCleanupCommandCount &&
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT ==
				metrics.iIngressHighWatermark,
			"Keep reliable ingress bounded while accepting, deduplicating, and priority-draining cleanup at hard cap");
	}
	{
		CServerApp app;
		auto sourceSimulation = std::make_shared<CGameRoom>(WORLD_ID::BERN);
		auto targetSimulation =
			std::make_shared<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		constexpr SESSION_ID transferSessionId = 41000u;
		auto session = std::make_shared<CClientSession>(
			transferSessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		app.m_Sessions.emplace(transferSessionId, session);
		app.m_SharedGameRooms.emplace(WORLD_ID::BERN, sourceSimulation);
		app.m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, targetSimulation);
		CServerApp::SESSION_GAMEPLAY_BINDING binding{};
		binding.eWorldId = WORLD_ID::BERN;
		binding.pSimulation = sourceSimulation;
		app.m_GameplayBindingBySessionId.emplace(transferSessionId, binding);

		bool filledTargetIngress = targetSimulation->Is_Ready();
		for (std::size_t index = 0u;
			index + 1u < CGameRoom::MAX_RELIABLE_COMMAND_COUNT; ++index)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
			command.iSessionId = static_cast<SESSION_ID>(50000u + index);
			command.UseSkill.iClientSequence = 1u;
			command.UseSkill.iSkillId = 34010u;
			filledTargetIngress = filledTargetIngress &&
				targetSimulation->Enqueue(std::move(command));
		}
		SERVER_WORLD_TRANSFER_REQUEST transfer{};
		transfer.iSessionId = transferSessionId;
		transfer.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
		transfer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		transfer.strNickName = "TransferRollbackFixture";
		sourceSimulation->m_PendingWorldTransfers.push_back(transfer);
		app.Handle_WorldTransfers(sourceSimulation);

		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			session->Get_CloseDiagnostic();
		const bool rollbackGuaranteed =
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW ==
				diagnostic.eReason &&
			WSAENOBUFS == diagnostic.iNativeErrorCode &&
			std::string::npos != diagnostic.strContext.find(
				"stage=target-enter-ingress") &&
			std::string::npos != diagnostic.strContext.find(
				"rollbackCleanupRequired=true") &&
			std::string::npos != diagnostic.strContext.find(
				"rollbackCleanupEnqueued=true") &&
			1u == targetSimulation->m_CleanupCommands.size() &&
			transferSessionId ==
				targetSimulation->m_CleanupCommands.front().iSessionId &&
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED ==
				targetSimulation->m_CleanupCommands.front().eLeaveReason &&
			std::none_of(
				targetSimulation->m_InboundCommands.begin(),
				targetSimulation->m_InboundCommands.end(),
				[](const ROOM_COMMAND& command)
				{
					return transferSessionId == command.iSessionId;
				});
		ROOM_COMMAND commandAfterCleanup{};
		commandAfterCleanup.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		commandAfterCleanup.iSessionId = transferSessionId;
		commandAfterCleanup.UseSkill.iClientSequence = 2u;
		commandAfterCleanup.UseSkill.iSkillId = 34010u;
		const bool commandRejectedBehindCleanup =
			!targetSimulation->Enqueue(std::move(commandAfterCleanup));
		targetSimulation->Tick(1.f / 30.f);
		tests.Require(
			filledTargetIngress && rollbackGuaranteed &&
			commandRejectedBehindCleanup &&
			targetSimulation->m_CleanupCommands.empty() &&
			targetSimulation->m_QueuedCleanupSessionIds.empty() &&
			app.m_GameplayBindingBySessionId.contains(transferSessionId) &&
			app.m_GameplayBindingBySessionId.at(transferSessionId).pSimulation ==
				sourceSimulation,
			"Classify transfer ingress failure and guarantee target rollback cleanup");
	}
	{
		constexpr const char* RAID_CLEAR_TEST_MODE_ENV =
			"LOSTARK_RAID_CLEAR_TEST_MODE";
		char* previousValue = nullptr;
		size_t previousValueLength = 0u;
		const bool hadPreviousValue = 0 == _dupenv_s(
			&previousValue, &previousValueLength, RAID_CLEAR_TEST_MODE_ENV) &&
			nullptr != previousValue;
		const std::string savedValue = hadPreviousValue ? previousValue : "";
		std::free(previousValue);
		(void)_putenv_s(RAID_CLEAR_TEST_MODE_ENV, "");

		auto addReturnPlayer = [](CGameRoom& room,
			const SESSION_ID sessionId, const PLAYER_ID playerId,
			const NET_ENTITY_ID entityId)
		{
			SERVER_PLAYER player{};
			player.iSessionId = sessionId;
			player.iPlayerId = playerId;
			player.iNetEntityId = entityId;
			player.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			player.strNickName = "RaidClearReturnFixture";
			player.iCurrentHp = 100u;
			player.iMaximumHp = 100u;
			player.isCombatReady = true;
			room.m_Players.emplace(playerId, player);
			room.m_PlayerIdBySessionId.emplace(sessionId, playerId);
			room.m_PlayerIdByEntityId.emplace(entityId, playerId);
		};

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID RETURN_SESSION = 42000u;
		addReturnPlayer(room, RETURN_SESSION, 42001u, 42002u);
		const bool encounterActivated =
			room.Activate_Encounter("boss.valtan.center");
		C2S_RETURN_TO_BERN request{};
		request.iRequestSequence = 1u;
		room.Handle_ReturnToBern(RETURN_SESSION, request);
		const bool rejectedBeforeClear = room.m_PendingWorldTransfers.empty();

		auto primaryBoss = std::find_if(
			room.m_WorldEntities.begin(), room.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"boss.valtan.center" == entity.strPlacementId;
			});
		const bool foundPrimaryBoss = primaryBoss != room.m_WorldEntities.end();
		if (foundPrimaryBoss)
		{
			primaryBoss->iCurrentHp = 0u;
			primaryBoss->eAction = SERVER_ENTITY_ACTION::DEAD;
			room.Update_WorldEntities(1.f / 30.f);
		}
		request.iRequestSequence = 2u;
		room.Handle_ReturnToBern(RETURN_SESSION, request);
		const bool acceptedAfterClear =
			room.m_bValtanRaidCleared &&
			1u == room.m_PendingWorldTransfers.size() &&
			WORLD_ID::BERN ==
				room.m_PendingWorldTransfers.front().eTargetWorldId &&
			"npc.bern.beda.guide" ==
				room.m_PendingWorldTransfers.front().strSpawnPlacementOverrideId;

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto previewRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& previewRoom = *previewRoomStorage;
		constexpr SESSION_ID PREVIEW_SESSION = 42100u;
		addReturnPlayer(previewRoom, PREVIEW_SESSION, 42101u, 42102u);
		(void)_putenv_s(RAID_CLEAR_TEST_MODE_ENV, "1");
		request.iRequestSequence = 1u;
		previewRoom.Handle_ReturnToBern(PREVIEW_SESSION, request);
		const bool explicitTestModeAccepted =
			1u == previewRoom.m_PendingWorldTransfers.size();

		(void)_putenv_s(RAID_CLEAR_TEST_MODE_ENV,
			hadPreviousValue ? savedValue.c_str() : "");
		tests.Require(
			room.Is_Ready() && encounterActivated && rejectedBeforeClear &&
			foundPrimaryBoss &&
			acceptedAfterClear,
			"Authorize Return to Bern only after the primary Product Valtan clear");
		tests.Require(
			previewRoom.Is_Ready() && explicitTestModeAccepted,
			"Allow an explicit Release Raid Clear test mode to exercise the Return button");
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
		CGameRoom& room = *roomStorage;
		const std::size_t commandCount =
			CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK + 5u;
		bool enqueued = room.Is_Ready();
		for (std::size_t index = 0u; index < commandCount; ++index)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
			command.iSessionId = static_cast<SESSION_ID>(50000u + index);
			command.UseSkill.iClientSequence = 1u;
			command.UseSkill.iSkillId = 34010u;
			const bool accepted = room.Enqueue(std::move(command));
			enqueued = enqueued && accepted;
			if (!accepted)
				break;
		}
		room.Tick(1.f / 30.f);
		const SERVER_ROOM_PERFORMANCE_METRICS firstTickMetrics =
			room.Get_PerformanceMetrics();
		const bool retainedFifoTail =
			5u == room.m_InboundCommands.size() &&
			static_cast<SESSION_ID>(
				50000u + CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK) ==
				room.m_InboundCommands.front().iSessionId;
		room.Tick(1.f / 30.f);
		const SERVER_ROOM_PERFORMANCE_METRICS secondTickMetrics =
			room.Get_PerformanceMetrics();

		SERVER_PLAYER player{};
		player.iSessionId = 60000u;
		player.iPlayerId = 60000u;
		player.iNetEntityId = 60000u;
		player.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		room.m_Players.emplace(player.iPlayerId, player);
		room.Broadcast_WorldSnapshot();
		const SERVER_ROOM_PERFORMANCE_METRICS snapshotMetrics =
			room.Get_PerformanceMetrics();
		tests.Require(
			enqueued && retainedFifoTail &&
			commandCount == firstTickMetrics.iLastIngressDepth &&
			CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK ==
				firstTickMetrics.iLastDrainedCommandCount &&
			5u == firstTickMetrics.iLastRemainingCommandCount &&
			1u == firstTickMetrics.iDrainLimitedTickCount &&
			5u == secondTickMetrics.iLastIngressDepth &&
			5u == secondTickMetrics.iLastDrainedCommandCount &&
			0u == secondTickMetrics.iLastRemainingCommandCount &&
			2u == secondTickMetrics.iTickCount &&
			1u == snapshotMetrics.iSnapshotEncodeCount &&
			0u == snapshotMetrics.iSnapshotEncodeFailureCount &&
			1u == snapshotMetrics.iSnapshotEnqueueBatchCount &&
			0u == snapshotMetrics.iSnapshotRecipientCount,
			"Drain a deterministic FIFO prefix per tick and record tick, ingress, encode, and enqueue metrics");
	}
	{
		using namespace LostArk::Shared::CombatCollision;

		const CIRCLE_XZ circle{ 0.f, 0.f, 2.f };
		const BODY_CIRCLE_XZ tangentCircle{ 3.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ missedCircle{ 3.001f, 0.f, 1.f };
		tests.Require(
			Circles_Overlap(circle, tangentCircle) &&
			!Circles_Overlap(circle, missedCircle),
			"Treat circle tangency as contact and reject a separated circle");

		const BODY_CIRCLE_XZ innerRingTangent{ 2.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ insideRingHole{ 1.9f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outerRingTangent{ 6.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outsideRing{ 6.001f, 0.f, 1.f };
		tests.Require(
			Circle_IntersectsRing(innerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(insideRingHole, 0.f, 0.f, 3.f, 5.f) &&
			Circle_IntersectsRing(outerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(outsideRing, 0.f, 0.f, 3.f, 5.f),
			"Respect both inclusive ring boundaries and reject both misses");

		const BODY_CIRCLE_XZ rotatedShapeHit{ 2.f, 2.f, 0.25f };
		const BODY_CIRCLE_XZ rotatedShapeMiss{ 0.f, 2.5f, 0.25f };
		tests.Require(
			Circle_IntersectsForwardBox(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsForwardBox(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a forward box in its rotated basis");
		tests.Require(
			Circle_IntersectsCone(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f) &&
			!Circle_IntersectsCone(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f),
			"Evaluate a cone in its rotated basis");
		tests.Require(
			Circle_IntersectsCross(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsCross(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a cross in its rotated basis");

		tests.Require(
			Circle_IntersectsEllipticSector({ 0.f, 9.f, .1f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 60.f) &&
			!Circle_IntersectsEllipticSector({ 1.1f, 8.f, .1f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 60.f) &&
			Circle_IntersectsEllipticSector({ 9.f, 0.f, .1f }, 0.f, 0.f, 1.f, 0.f, 1.f, 10.f, 60.f),
			"Elliptic sectors preserve independent width/depth and rotate with boss facing");
		tests.Require(
			!Circle_IntersectsEllipticSector({ 0.f, 7.f, .1f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 60.f, true) &&
			Circle_IntersectsEllipticSector({ 0.f, -7.f, .1f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 60.f, true) &&
			!Circle_IntersectsEllipticSector({ 1.3f, 0.f, .2f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 360.f) &&
			Circle_IntersectsEllipticSector({ 1.2f, 0.f, .2f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 360.f),
			"Reverse sector leaves the safe wedge and ellipse contact uses the actual body radius");
		const float ellipseTheta = .6f;
		const float arcX = std::sin(ellipseTheta), arcZ = 10.f * std::cos(ellipseTheta);
		const float normalLength = std::sqrt(arcX * arcX + arcZ * arcZ / 10000.f);
		const float normalX = arcX / normalLength, normalZ = arcZ / (100.f * normalLength);
		tests.Require(
			Circle_IntersectsEllipticSector({ arcX + .25f * normalX, arcZ + .25f * normalZ, .25f },
				0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 120.f) &&
			!Circle_IntersectsEllipticSector({ arcX + .251f * normalX, arcZ + .251f * normalZ, .25f },
				0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 120.f),
			"Ellipse arc tangency is inclusive and a separated body misses away from the principal axes");
		tests.Require(
			!Circle_IntersectsEllipticSector({ 0.f, 0.f, .1f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 0.f) &&
			Circle_IntersectsEllipticSector({ 0.f, 7.f, .1f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 0.f, true) &&
			!Circle_IntersectsEllipticSector({ 0.f, 0.f, .1f }, 0.f, 0.f, 0.f, 1.f, 1.f, 10.f, 360.f, true) &&
			!Circle_IntersectsEllipticSector({ 0.f, 0.f, .1f }, 0.f, 0.f, 0.f, 1.f, 0.f, 10.f, 60.f),
			"Elliptic sector empty/full complement and invalid radii are explicit");

		constexpr float ROOT_THREE_OVER_TWO = 0.8660254f;
		const std::array<BODY_CIRCLE_XZ, 6u> sixDirectionArms = {{
			{ 0.f, 5.f, 0.2f },
			{ 0.f, -5.f, 0.2f },
			{ 5.f * ROOT_THREE_OVER_TWO, 2.5f, 0.2f },
			{ -5.f * ROOT_THREE_OVER_TWO, -2.5f, 0.2f },
			{ -5.f * ROOT_THREE_OVER_TWO, 2.5f, 0.2f },
			{ 5.f * ROOT_THREE_OVER_TWO, -2.5f, 0.2f }
		}};
		const std::array<BODY_CIRCLE_XZ, 6u> sixDirectionGaps = {{
			{ 2.5f, 5.f * ROOT_THREE_OVER_TWO, 0.2f },
			{ -2.5f, -5.f * ROOT_THREE_OVER_TWO, 0.2f },
			{ 5.f, 0.f, 0.2f },
			{ -5.f, 0.f, 0.2f },
			{ 2.5f, -5.f * ROOT_THREE_OVER_TWO, 0.2f },
			{ -2.5f, 5.f * ROOT_THREE_OVER_TWO, 0.2f }
		}};
		const auto hitsSixDirections = [](const BODY_CIRCLE_XZ& body)
		{
			return Circle_IntersectsSixDirections(
				body, 0.f, 0.f, 0.f, 1.f, 6.f, 0.5f);
		};
		tests.Require(
			std::all_of(
				sixDirectionArms.begin(), sixDirectionArms.end(),
				hitsSixDirections) &&
			std::none_of(
				sixDirectionGaps.begin(), sixDirectionGaps.end(),
				hitsSixDirections),
			"Hit all six centered-strip arms and preserve all six angular gaps");
	}
}

