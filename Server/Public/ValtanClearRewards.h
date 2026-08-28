#pragma once

#include <string>
#include <vector>

namespace LostArk::Server
{
	// Published Data/Valtan/Valtan.clearrewards.json -- the ordered set of item
	// IDs a room grants to every player present the moment Valtan's own eAction
	// first reaches DEAD (see CGameRoom's world entity tick loop). Order matters:
	// it is also the order the Client queues its "you got X" announcements in.
	class CValtanClearRewards final
	{
	public:
		bool Load();

		const std::vector<std::string>& Get_ItemIds() const { return m_ItemIds; }

		const std::string& Get_Status() const { return m_strStatus; }

	private:
		std::vector<std::string> m_ItemIds;
		std::string m_strStatus;
	};
}
