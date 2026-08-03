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
		END
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
