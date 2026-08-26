#pragma once

#include "Network/PacketType.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	enum class WORLD_BOOTSTRAP_KIND
	{
		PLAYER_SPAWN,
		NPC,
		BOSS,
		MONSTER,
		TRIGGER_BOX,
		COLLISION_BOX,
		END
	};

	enum class WORLD_TRIGGER_ACTION_KIND
	{
		MOVE_PLAYER,
		CHANGE_LEVEL,
		ACTIVATE_SPAWN_GROUP,
		ACTIVATE_ENCOUNTER,
		END
	};

	enum class NPC_BEHAVIOR_MODE : std::uint8_t
	{
		STATIONARY,
		PATROL,
		WANDER,
		END
	};

	enum class NPC_ROUTE_MODE : std::uint8_t
	{
		LOOP,
		PING_PONG,
		ONCE,
		END
	};

	enum class NPC_ACTION_SELECTION : std::uint8_t
	{
		SEQUENCE,
		WEIGHTED,
		END
	};

	struct WORLD_NPC_BEHAVIOR_WAYPOINT final
	{
		std::string strWaypointId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		std::uint32_t iWaitMs = 0u;
		bool bHasLookYaw = false;
		float fLookYawDegrees = 0.f;
	};

	struct WORLD_NPC_BEHAVIOR_ACTION final
	{
		std::string strActionId;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iWaitAfterMs = 0u;
		std::uint32_t iWeight = 1u;
	};

	/* Server-only logical behavior compiled from Gameplay.world.json. Actual
	clip names, loop flags, playback rates and blend values belong to the Client
	presentation document and must never cross this descriptor. */
	struct WORLD_NPC_BEHAVIOR_DESCRIPTOR final
	{
		NPC_BEHAVIOR_MODE eMode = NPC_BEHAVIOR_MODE::STATIONARY;
		NPC_ROUTE_MODE eRouteMode = NPC_ROUTE_MODE::LOOP;
		NPC_ACTION_SELECTION eActionSelection =
			NPC_ACTION_SELECTION::SEQUENCE;
		float fMoveSpeed = 1.f;
		float fWanderRadius = 0.f;
		std::uint32_t iRandomSeed = 1u;
		std::uint32_t iStartDelayMs = 0u;
		std::uint32_t iIdleMinMs = 0u;
		std::uint32_t iIdleMaxMs = 0u;
		std::string strLookTargetPlacementId;
		std::vector<WORLD_NPC_BEHAVIOR_WAYPOINT> Waypoints;
		std::vector<WORLD_NPC_BEHAVIOR_ACTION> Actions;
	};

	struct WORLD_TRIGGER_ACTION
	{
		WORLD_TRIGGER_ACTION_KIND eKind = WORLD_TRIGGER_ACTION_KIND::END;
		float fTargetX = 0.f;
		float fTargetY = 0.f;
		float fTargetZ = 0.f;
		float fDurationSeconds = 0.f;
		float fArcHeight = 0.f;
		LostArk::Shared::WORLD_ID eTargetWorldId =
			LostArk::Shared::WORLD_ID::END;
		std::string strTargetId;
	};

	struct WORLD_BOOTSTRAP_PLACEMENT
	{
		std::string strPlacementId;
		WORLD_BOOTSTRAP_KIND eKind = WORLD_BOOTSTRAP_KIND::END;
		std::string strArchetypeId;
		std::string strEncounterId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		float fHalfExtentX = 0.f;
		float fHalfExtentY = 0.f;
		float fHalfExtentZ = 0.f;
		bool isTriggerOnce = true;
		std::vector<WORLD_TRIGGER_ACTION> TriggerActions;
		bool bHasNpcBehavior = false;
		WORLD_NPC_BEHAVIOR_DESCRIPTOR NpcBehavior;
		bool isEnabled = true;
	};

	class CWorldBootstrap final
	{
	public:
		bool Load(LostArk::Shared::WORLD_ID worldId);

		const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& Get_Placements() const
		{
			return m_Placements;
		}

		const std::string& Get_AreaId() const { return m_strAreaId; }
		const std::string& Get_Status() const { return m_strStatus; }
		std::uint32_t Get_Revision() const { return m_iRevision; }

	private:
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> m_Placements;
		std::string m_strAreaId;
		std::string m_strStatus;
		std::uint32_t m_iRevision = 0;
	};
}
