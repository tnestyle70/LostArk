#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
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
			// Exercise the actual caller: a wire-only round trip cannot detect lost staging.
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			auto& player = room->m_Players[1u];
			player.iPlayerId = 1u;
			player.iCurrentHp = player.iMaximumHp = 100u;
			bool playAndReplayPlace = room->Is_Ready();
			bool stopAndMotionPreserve = room->Is_Ready();
			const WORLD_SEQUENCE_OPERATION operations[] = { WORLD_SEQUENCE_OPERATION::PLAY,
				WORLD_SEQUENCE_OPERATION::REPLAY, WORLD_SEQUENCE_OPERATION::STOP, WORLD_SEQUENCE_OPERATION::PLAY };
			for (unsigned scenario = 0u; scenario < 4u; ++scenario)
			{
				player.fPositionX = 12.f; player.fPositionY = 34.f; player.fPositionZ = 56.f;
				player.hasMoveGoal = true; player.TriggerMove.isActive = true;
				player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
				player.iMarioStage = 1u; player.ePreMarioForm = PLAYER_MADNESS_FORM::NORMAL;
				player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
				room->Broadcast_WorldSequencePlay("world.sequence.instance.original_kouku", 1.f, 0.f, 0.f, 0.f, 0u,
					scenario == 3u ? "world.existing.target" : "", operations[scenario]);
				if (scenario < 2u)
					playAndReplayPlace = playAndReplayPlace &&
						std::abs(player.fPositionX + 1.166f) < .0001f && std::abs(player.fPositionY - 1.31f) < .0001f &&
						std::abs(player.fPositionZ - 745.078f) < .0001f && !player.hasMoveGoal && !player.TriggerMove.isActive &&
						player.eAction == PLAYER_ACTION_STATE::NONE && player.iMarioStage == 0u &&
						player.eMadnessForm == PLAYER_MADNESS_FORM::NORMAL && player.iCurrentHp == 100u;
				else
					stopAndMotionPreserve = stopAndMotionPreserve &&
						player.fPositionX == 12.f && player.fPositionY == 34.f && player.fPositionZ == 56.f &&
						player.hasMoveGoal && player.TriggerMove.isActive && player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE &&
						player.iMarioStage == 1u && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN && player.iCurrentHp == 100u;
			}
			tests.Require(playAndReplayPlace, "Book PLAY and REPLAY both stage the party and retire prior movement");
			tests.Require(stopAndMotionPreserve, "Book STOP and exact target motion preserve party position and movement");
		}
		std::cout << "World playback contract failures: " << tests.failures << '\n';
		return tests.failures == 0 ? 0 : 1;
	}
