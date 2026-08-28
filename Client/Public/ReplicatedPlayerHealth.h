#pragma once

#include "ActionPresentationTimeline.h"
#include "Network/PacketMessages.h"

#include <unordered_map>

namespace Client
{

struct REPLICATED_PLAYER_HEALTH final
{
	bool hasSnapshot = false;
	std::uint32_t iCurrentHp = 0u;
	std::uint32_t iMaximumHp = 0u;

	float Get_Ratio() const
	{
		return hasSnapshot && iMaximumHp > 0 ?
			static_cast<float>(iCurrentHp) / static_cast<float>(iMaximumHp) : 0.f;
	}
};

// Read-only presentation join keyed by the Server entity identity. Replacing
// the complete snapshot also removes HP for players no longer in this world.
class CReplicatedPlayerHealth final
{
public:
	bool Apply_Snapshot(const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot)
	{
		if (0u == snapshot.iServerTick)
			return false;
		if (!CActionPresentationTimeline::Is_ForwardTick(snapshot.iServerTick, m_iServerTick))
			return true;
		decltype(m_ByNetEntityId) staged;
		for (const auto& player : snapshot.Players)
		{
			if (LostArk::Shared::INVALID_NET_ENTITY_ID == player.iNetEntityId ||
				0u == player.iMaximumHp ||
				player.iCurrentHp > player.iMaximumHp ||
				!staged.emplace(player.iNetEntityId, REPLICATED_PLAYER_HEALTH{
					true, player.iCurrentHp, player.iMaximumHp }).second)
			{
				return false;
			}
		}
		m_ByNetEntityId = std::move(staged);
		m_iServerTick = snapshot.iServerTick;
		return true;
	}

	REPLICATED_PLAYER_HEALTH Find(LostArk::Shared::NET_ENTITY_ID entityId) const
	{
		const auto found = m_ByNetEntityId.find(entityId);
		return m_ByNetEntityId.end() != found ? found->second : REPLICATED_PLAYER_HEALTH{};
	}
	void Erase(LostArk::Shared::NET_ENTITY_ID entityId) { m_ByNetEntityId.erase(entityId); }
	void Reset() { m_ByNetEntityId.clear(); m_iServerTick = 0u; }

private:
	std::unordered_map<LostArk::Shared::NET_ENTITY_ID, REPLICATED_PLAYER_HEALTH> m_ByNetEntityId;
	std::uint32_t m_iServerTick = 0u;
};

}
