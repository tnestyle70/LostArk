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
		std::string strPatternId;
		std::string strActionId;
		std::string strDamageProfileId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		float fPatternMinimumRange = 0.f;
		float fPatternMaximumRange = 0.f;
		std::uint32_t iPatternTelegraphMs = 0;
		std::uint32_t iPatternActiveMs = 0;
		std::uint32_t iPatternRecoveryMs = 0;
		float fHalfExtentX = 0.f;
		float fHalfExtentY = 0.f;
		float fHalfExtentZ = 0.f;
		bool isTriggerOnce = true;
		std::vector<WORLD_TRIGGER_ACTION> TriggerActions;
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
