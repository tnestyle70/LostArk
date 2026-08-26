#pragma once

#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	class CServerNavigation;
	class CServerCollisionSystem;
	struct SERVER_WORLD_ENTITY;

	enum class SERVER_NPC_BEHAVIOR_PHASE : std::uint8_t
	{
		START_DELAY,
		IDLE_WAIT,
		MOVING,
		AMBIENT_ACTION,
		COMPLETE
	};

	/* Mutable state only. The authored route and action definitions stay in the
	world bootstrap placement and are looked up by stable placement ID. */
	struct SERVER_NPC_BEHAVIOR_STATE final
	{
		NPC_BEHAVIOR_MODE eMode = NPC_BEHAVIOR_MODE::STATIONARY;
		SERVER_NPC_BEHAVIOR_PHASE ePhase =
			SERVER_NPC_BEHAVIOR_PHASE::START_DELAY;
		std::size_t iWaypointIndex = 0u;
		std::size_t iActionIndex = 0u;
		std::uint32_t iWaitUntilTick = 0u;
		std::uint32_t iRandomState = 1u;
		std::uint32_t iPendingWaitMs = 0u;
		std::uint64_t iNavigationRevision = 0u;
		bool bRouteForward = true;
		bool bRouteComplete = false;
		bool bInitialized = false;
	};

	class CNpcBehaviorRuntime final
	{
	public:
		static constexpr const char* IDLE_ACTION_ID = "npc.idle";
		static constexpr const char* WALK_ACTION_ID = "npc.move.walk";

		bool Validate_Admission(
			const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
			const CServerNavigation& navigation,
			std::string& outStatus) const;

		bool Initialize(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			const CServerNavigation& navigation,
			std::uint32_t startTick,
			SERVER_WORLD_ENTITY& entity,
			std::string& outStatus) const;

		bool Update(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			const SERVER_WORLD_ENTITY* lookTarget,
			const CServerNavigation& navigation,
			const CServerCollisionSystem& collision,
			float fixedDeltaSeconds,
			std::uint32_t updateTick,
			SERVER_WORLD_ENTITY& entity,
			std::string& outStatus) const;

	private:
		static bool Validate_Descriptor(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			const CServerNavigation& navigation,
			std::string& outStatus);
	};
}
