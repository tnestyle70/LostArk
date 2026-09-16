#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
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

int LostArk::Server::CServerGameplayContractRunner::Run_WorldPlayback(TESTS& tests)
{
		CWorldBootstrap bootstrap;
		tests.Require(bootstrap.Load(WORLD_ID::KAKULSAYDON_ARENA) && !bootstrap.Get_SequenceInstanceIds().empty(),
			"Viewer loads published Kouku sequence IDs with the world");
		tests.Require(bootstrap.Load(WORLD_ID::VALTAN_ARENA) && bootstrap.Get_SequenceInstanceIds().empty(),
			"Viewer switching to Valtan clears the previous world's sequence IDs");
		CServerTriggerSystem triggers;
		WORLD_BOOTSTRAP_PLACEMENT box{};
		box.strPlacementId = "viewer.trigger"; box.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		box.fHalfExtentX = box.fHalfExtentY = box.fHalfExtentZ = 1.f;
		box.isTriggerOnce = true; box.requiresInteract = true;
		WORLD_TRIGGER_ACTION action{}; action.eKind = WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
		action.strTargetId = "viewer.sequence"; box.TriggerActions.push_back(action);
		std::string status;
		tests.Require(triggers.Initialize({ box }, status), "Viewer test initializes the real trigger system");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		players[1u].iPlayerId = 1u; players[1u].iCurrentHp = players[1u].iMaximumHp = 100u;
		players[1u].fPositionX = 50.f;
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		int fired = 0;
		const auto activate = [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
		{ if (kind != WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE || id != "viewer.sequence") return false; ++fired; return true; };
		using R = DEBUG_WORLD_PLAYBACK_RESULT;
		tests.Require(triggers.Debug_Activate(2u, box.strPlacementId, false, players, 1u, transfers, activate) ==
#ifdef _DEBUG
			R::INVALID_PLAYER,
#else
			R::DISABLED,
#endif
			"Viewer rejects missing player without activating a trigger");
#ifdef _DEBUG
		tests.Require(triggers.Debug_Activate(1u, "missing", false, players, 1u, transfers, activate) == R::INVALID_TARGET && fired == 0,
			"Viewer rejects unknown targets without effects");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 1u, transfers, activate) == R::ACCEPTED && fired == 1,
			"Debug viewer uses the authored action outside the G-key box");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 2u, transfers, activate) == R::ALREADY_USED && fired == 1,
			"Play preserves the one-shot latch");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 3u, transfers, activate) == R::ACCEPTED && fired == 2,
			"Replay reuses the same authored action");
		players[1u].iCurrentHp = 0;
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 4u, transfers, activate) == R::INVALID_PLAYER && fired == 2,
			"Dead viewer cannot activate world actions");
		players[1u].iCurrentHp = 100;
		const auto reject = [](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return false; };
		tests.Require(triggers.Initialize({ box }, status) &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 5u, transfers, reject) == R::ACTION_REJECTED &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 6u, transfers, activate) == R::ACCEPTED,
			"Failed action does not consume the one-shot trigger");
#endif
		{
			// Exercise the real broadcast boundary: stale bootstrap rows cannot revive
			// the old actor or move the party before the Client rejects the cue.
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			auto& player = room->m_Players[1u];
			player.iPlayerId = 1u;
			player.iCurrentHp = player.iMaximumHp = 100u;
			bool allPreserve = room->Is_Ready();
			bool admissionMatches = room->Is_Ready();
			const WORLD_SEQUENCE_OPERATION operations[] = { WORLD_SEQUENCE_OPERATION::PLAY,
				WORLD_SEQUENCE_OPERATION::REPLAY, WORLD_SEQUENCE_OPERATION::STOP,
				WORLD_SEQUENCE_OPERATION::PLAY, WORLD_SEQUENCE_OPERATION::PLAY };
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				player.fPositionX = 12.f; player.fPositionY = 34.f; player.fPositionZ = 56.f;
				player.hasMoveGoal = true; player.TriggerMove.isActive = true;
				player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
				player.iMarioStage = 1u; player.ePreMarioForm = PLAYER_MADNESS_FORM::NORMAL;
				player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
				const bool accepted = room->Broadcast_WorldSequencePlay(
					scenario == 4u ? "world.sequence.instance.circusfinale" : "world.sequence.instance.original_kouku",
					1.f, 0.f, 0.f, 0.f, 0u, scenario == 3u ? "world.existing.target" : "", operations[scenario]);
				admissionMatches = admissionMatches && accepted == (scenario == 2u || scenario == 4u);
				allPreserve = allPreserve &&
					player.fPositionX == 12.f && player.fPositionY == 34.f && player.fPositionZ == 56.f &&
					player.hasMoveGoal && player.TriggerMove.isActive && player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE &&
					player.iMarioStage == 1u && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN && player.iCurrentHp == 100u;
			}
			tests.Require(admissionMatches, "Legacy PLAY/REPLAY/motion reject; STOP and other sequences remain admitted");
			tests.Require(allPreserve, "Legacy rejection and ordinary sequence cues preserve all player movement and form state");
		}
		{
			C2S_DEBUG_WORLD_PLAYBACK request{};
			request.iRequestSequence = 17u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
			request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
			request.strOccurrenceId = request.strTargetId + ".logic.51";
			request.iRunEpoch = 9u; request.iRoomPlayerSlot = 3u;
			request.fPositionX = -1.156042f; request.fPositionY = 1.3176255f; request.fPositionZ = 742.512031f;
			CPacketWriter writer;
			tests.Require(Write_Message(writer, request), "Arrival command encodes its run, occurrence, slot and destination");
			CPacketReader reader(writer.Get_Buffer()); C2S_DEBUG_WORLD_PLAYBACK decoded;
			tests.Require(Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 9u && decoded.iRoomPlayerSlot == 3u && decoded.strOccurrenceId == request.strOccurrenceId &&
				decoded.fPositionX == request.fPositionX && decoded.fPositionY == request.fPositionY && decoded.fPositionZ == request.fPositionZ,
				"Arrival packet preserves exact slot coordinates and replay identity");
			auto bytes = writer.Get_Buffer(); bytes.pop_back();
			CPacketReader truncated(bytes); decoded.strOccurrenceId = "sentinel";
			tests.Require(!Read_Message(truncated, decoded) && decoded.strOccurrenceId == "sentinel",
				"Truncated arrival packet does not partially commit decoded intent");
			bool rejects = true;
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				auto bad = request;
				if (scenario == 0u) bad.iRoomPlayerSlot = 4u;
				if (scenario == 1u) bad.iRunEpoch = 0u;
				if (scenario == 2u) bad.fPositionX = std::numeric_limits<float>::quiet_NaN();
				if (scenario == 3u) bad.eWorldId = WORLD_ID::BERN;
				if (scenario == 4u) bad.fPositionZ = 100001.f;
				CPacketWriter invalid; rejects = rejects && !Write_Message(invalid, bad) && invalid.Get_Buffer().empty();
			}
			tests.Require(rejects, "Arrival rejects wrong world, invalid epoch, slot and coordinates before writing");
			S2C_DEBUG_WORLD_PLAYBACK_RESULT receipt{};
			receipt.iRequestSequence = request.iRequestSequence; receipt.eWorldId = request.eWorldId;
			receipt.eOperation = request.eOperation; receipt.strTargetId = request.strTargetId;
			receipt.eResult = R::SKIPPED_PLAYER;
			CPacketWriter replyWriter; const bool wroteReply = Write_Message(replyWriter, receipt);
			CPacketReader replyReader(replyWriter.Get_Buffer()); S2C_DEBUG_WORLD_PLAYBACK_RESULT reply;
			tests.Require(wroteReply && Read_Message(replyReader, reply) && replyReader.Get_RemainingSize() == 0u &&
				reply.iRequestSequence == 17u && reply.eOperation == request.eOperation && reply.eResult == R::SKIPPED_PLAYER,
				"Arrival skip reply uses the existing request-correlated result envelope");
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLAY_SEQUENCE;
			CPacketWriter legacyWriter; const bool wroteLegacy = Write_Message(legacyWriter, request);
			CPacketReader legacyReader(legacyWriter.Get_Buffer());
			tests.Require(wroteLegacy && Read_Message(legacyReader, decoded) && legacyReader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 0u && decoded.strOccurrenceId.empty(), "Ordinary world playback keeps its original payload shape");
		}
#ifdef _DEBUG
		{
			WSADATA winsock{};
			const bool socketReady = WSAStartup(MAKEWORD(2, 2), &winsock) == 0;
			tests.Require(socketReady, "Arrival fixture prepares unconnected session sockets without a listener");
			if (socketReady)
			{
				const std::array<std::array<float, 2>, 4> locations{{ {-3.913588f,739.883125f},
					{-3.290587f,742.070391f}, {-5.324063f,738.528281f}, {-1.156042f,742.512031f} }};
				for (unsigned count = 1u; count <= 4u; ++count)
				{
					auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
					std::vector<std::shared_ptr<CClientSession>> sessions;
					const auto join = [&](PLAYER_ID id)
					{
						const SESSION_ID sessionId = id + 1000u;
						auto connection = std::make_shared<CClientSession>(sessionId, ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
							CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
						sessions.push_back(connection); room->m_Sessions[sessionId] = connection;
						room->m_PlayerIdBySessionId[sessionId] = id;
						auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = sessionId;
						player.iNetEntityId = id + 100u; player.iCurrentHp = player.iMaximumHp = 100u;
						player.fPositionX = -100.f - static_cast<float>(id); player.fPositionY = 1.3f; player.fPositionZ = 740.f;
						player.hasMoveGoal = true; player.iCurrentSkillId = 34010u; player.eAction = PLAYER_ACTION_STATE::SKILL;
					};
					// Reverse insertion proves that stable PlayerId order, not joins or session order, chooses slots.
					for (unsigned n = count; n > 0u; --n) join(n * 10u);
					if (count == 4u) join(50u);
					bool nativeGround = room->Is_Ready();
					std::array<SERVER_NAV_POINT, 4> ground{};
					for (unsigned slot = 0u; slot < 4u; ++slot)
						nativeGround = nativeGround && room->m_ServerNavigation.Sample_Position(locations[slot][0], locations[slot][1], ground[slot]);
					tests.Require(nativeGround, "Arrival samples all four authored fireworks XZ on actual Server navigation");
					if (!nativeGround) continue;
					C2S_DEBUG_WORLD_PLAYBACK request{};
					request.iRequestSequence = 1u; request.iRunEpoch = 1u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
					request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
					const auto position = [&](unsigned slot)
					{
						request.iRoomPlayerSlot = static_cast<std::uint8_t>(slot);
						request.strOccurrenceId = request.strTargetId + ".logic." + std::to_string(slot + 1u);
						request.fPositionX = ground[slot].x; request.fPositionY = ground[slot].y; request.fPositionZ = ground[slot].z;
					};
					bool movedInOrder = true;
					for (unsigned slot = 0u; slot < 4u; ++slot)
					{
						position(slot); const auto verdict = room->Apply_DebugRoomPlayerArrival(1010u, request);
						movedInOrder = movedInOrder && verdict == (slot < count ? R::ACCEPTED : R::SKIPPED_PLAYER);
						if (slot < count)
						{
							const auto& player = room->m_Players.at((slot + 1u) * 10u);
							movedInOrder = movedInOrder && player.fPositionX == ground[slot].x && player.fPositionY == ground[slot].y &&
								player.fPositionZ == ground[slot].z && !player.hasMoveGoal && player.iCurrentSkillId == INVALID_SKILL_ID && player.iCurrentHp == 100u;
						}
					}
					tests.Require(movedInOrder, "One through four connected players arrive by PlayerId; missing slots are successful skips");
					if (count == 4u)
						tests.Require(room->m_RoomPlayerArrivalRuns.at(1010u).Players.size() == 4u && room->m_Players.at(50u).hasMoveGoal,
							"Arrival roster caps at four and preserves any later connected player");
					position(0u); auto& first = room->m_Players.at(10u); first.fPositionX -= 20.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ALREADY_USED && first.fPositionX == ground[0].x - 20.f,
						"Duplicate arrival occurrence never teleports an already consumed slot twice");
					request.iRunEpoch = 2u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACCEPTED && first.fPositionX == ground[0].x,
						"Explicit new playback epoch permits the same occurrence again");
					first.fPositionX -= 20.f; first.hasMoveGoal = true; first.iCurrentSkillId = 34010u;
					request.iRunEpoch = 1u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::STALE_REQUEST && first.hasMoveGoal && first.iCurrentSkillId == 34010u,
						"An older playback cannot mutate the current run");
					request.iRunEpoch = 2u; request.strOccurrenceId += ".wrongheight"; request.fPositionY += 100.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACTION_REJECTED && first.fPositionX == ground[0].x - 20.f &&
						first.hasMoveGoal && first.iCurrentSkillId == 34010u && first.iCurrentHp == 100u,
						"Rejected destination preserves position, action, movement and health");
					request.eWorldId = WORLD_ID::BERN;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::WRONG_WORLD && first.hasMoveGoal,
						"Arrival cannot cross the requesting room's world boundary");
					request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					if (count > 1u)
					{
						// Epoch 2 already captured PlayerId 20; replacing its room binding cannot retarget that slot.
						room->m_PlayerIdBySessionId.erase(1020u); room->m_Players.erase(20u); join(21u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(21u).hasMoveGoal,
							"Departed roster member is skipped without teleporting its newly joined replacement");
					}
					else
					{
						join(20u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(20u).hasMoveGoal,
							"Joining midway does not fill a slot absent from the playback's fixed roster");
					}
				}
				WSACleanup();
			}
		}
#endif
		std::cout << "World playback contract failures: " << tests.failures << '\n';
		return tests.failures == 0 ? 0 : 1;
	}
