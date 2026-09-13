#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanReleaseControl(TESTS& tests)
{
#ifndef _DEBUG
	{
		/* The wire contract remains known in Release so a Debug Client receives
		an explicit verdict, but neither start nor control may stage room state. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START start{};
		start.iRequestSequence = 1u;
		start.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		start.strBossPlacementId = "boss.valtan.center";
		start.strFlowId = "flow.valtan.release-contract";
		start.strFlowRevision = std::string(64u, 'a');
		start.strStartSlotId = "flow.slot.000001";
		start.iInterStepPursuitMs =
			MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS;
		start.Slots = {
			{ "flow.slot.000001", "VALTAN_WHIRLWIND" }
		};
		const std::string statusBefore = room.m_strStatus;
		const std::size_t worldEntityCountBefore = room.m_WorldEntities.size();
		std::uint32_t reportedEpoch = 123u;
		GameplayDataRevision reportedRevision{};
		reportedRevision.Bytes.front() = 1u;
		std::string reason;
		const VALTAN_PATTERN_FLOW_RESULT startResult =
			room.Evaluate_ValtanPatternFlowStart(
				1u, start, reportedEpoch, reportedRevision, reason);

		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT stop{};
		stop.iControlSequence = 1u;
		stop.strFlowId = start.strFlowId;
		stop.iRoomFlowEpoch = 1u;
		std::uint32_t stopEpoch = 123u;
		GameplayDataRevision stopRevision{};
		stopRevision.Bytes.front() = 1u;
		std::string stopReason;
		const VALTAN_PATTERN_FLOW_RESULT stopResult =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				1u, stop, stopEpoch, stopRevision, stopReason);

		tests.Require(
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD == startResult &&
			0u == reportedEpoch && !reportedRevision.Is_Valid() &&
			!reason.empty() &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD == stopResult &&
			0u == stopEpoch && !stopRevision.Is_Valid() &&
			!stopReason.empty() && statusBefore == room.m_strStatus &&
			worldEntityCountBefore == room.m_WorldEntities.size(),
			"Reject Boss Tool pattern-flow start and stop commands in a Release Server without staging room state");
	}
#endif
}

