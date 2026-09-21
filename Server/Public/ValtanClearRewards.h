#pragma once

#include "Network/PacketType.h"

#include <array>
#include <string>
#include <vector>

namespace LostArk::Server
{
	// Published Data/Valtan/Valtan.clearrewards.json -- per class, the ordered set
	// of item IDs a room grants to each player present the moment Valtan's own
	// eAction first reaches DEAD (see CGameRoom's world entity tick loop). Order
	// matters: it is also the order the Client queues its "you got X"
	// announcements in. A class with no list gets nothing.
	class CValtanClearRewards final
	{
	public:
		bool Load();

		const std::vector<std::string>& Get_ItemIds(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;

		const std::string& Get_Status() const { return m_strStatus; }

	private:
		using CLASS_ITEM_IDS = std::array<std::vector<std::string>,
			static_cast<std::size_t>(LostArk::Shared::CHARACTER_CLASS_ID::END)>;
		CLASS_ITEM_IDS m_ItemIds;
		std::string m_strStatus;
	};
}
