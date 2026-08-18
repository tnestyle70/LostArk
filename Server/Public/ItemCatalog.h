#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace LostArk::Server
{
	// One row of the published Data/Items/ItemCatalog.json. Debug-only slice:
	// the Server only needs enough to validate a give-item request and cap a
	// stack, so no display name or category travels through the bootstrap.
	struct SERVER_ITEM_DEFINITION
	{
		std::string strItemId;
		std::uint32_t iMaxStack = 0;
		// 0 for a non-consumable; otherwise the percent of maximum HP a single
		// use restores (e.g. the three HP potion tiers: 15/30/45).
		std::uint32_t iHealPercent = 0;
	};

	class CItemCatalog final
	{
	public:
		bool Load();

		const SERVER_ITEM_DEFINITION* Find_Item(
			const std::string& itemId) const;

		const std::string& Get_Status() const { return m_strStatus; }

	private:
		std::unordered_map<std::string, SERVER_ITEM_DEFINITION> m_Items;
		std::string m_strStatus;
	};
}
