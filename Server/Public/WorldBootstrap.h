#pragma once

#include "Network/PacketMessages.h"

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
		/* Starts one authored world sequence instance. The Server owns the OBB
		   entry that decides when; the sequence itself is Client presentation. */
		PLAY_SEQUENCE,
		/* Claims the card maze telescope for the interacting player. The room
		   deals the suits and raises the targets; only an interact-gated box may
		   carry it, because an entry edge has no claimant. */
		CLAIM_CARD_MAZE_TELESCOPE,
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
		LostArk::Shared::KOUKU_HUD_MODE eKoukuHudModeOnArrival = LostArk::Shared::KOUKU_HUD_MODE::END;
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
		/* False runs the actions the moment a player's body enters the box.
		   True holds them until that player asks, so the Server offers a prompt
		   on entry and runs nothing until the request arrives. */
		bool requiresInteract = false;
		std::vector<WORLD_TRIGGER_ACTION> TriggerActions;
		bool bHasNpcBehavior = false;
		WORLD_NPC_BEHAVIOR_DESCRIPTOR NpcBehavior;
		bool isEnabled = true;
	};

	/* Geometry/timing compiled from the 36 authored Seto lanes; no asset paths. */
	struct CARD_MAZE_MARCH_LANE final
	{
		std::string instanceId;
		std::uint32_t delayMs = 0u, durationMs = 0u;
		float startX = 0.f, startY = 0.f, startZ = 0.f;
		float endX = 0.f, endY = 0.f, endZ = 0.f;
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
		const std::vector<std::string>& Get_SequenceInstanceIds() const { return m_SequenceInstanceIds; }
		const std::vector<CARD_MAZE_MARCH_LANE>& Get_CardMazeLanes() const { return m_CardMazeLanes; }

	private:
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> m_Placements;
		std::vector<std::string> m_SequenceInstanceIds;
		std::vector<CARD_MAZE_MARCH_LANE> m_CardMazeLanes;
		std::string m_strAreaId;
		std::string m_strStatus;
		std::uint32_t m_iRevision = 0;
	};
}
