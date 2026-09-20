#pragma once

#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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
		// ItemCatalog.json equipSlot ("helmet", "earring"...) and characterClass
		// ("LanceMaster"...); empty when the item is not equipment / not class-bound.
		std::string strEquipSlot;
		std::string strCharacterClass;
		// The slot a fresh character already wears this item in; NONE otherwise.
		LostArk::Shared::EQUIPMENT_SLOT eStartingEquippedSlot =
			LostArk::Shared::EQUIPMENT_SLOT::NONE;
	};

	class CItemCatalog final
	{
	public:
		bool Load();

		const SERVER_ITEM_DEFINITION* Find_Item(
			const std::string& itemId) const;

		const std::string& Get_Status() const { return m_strStatus; }

		/* Every (item, slot) a fresh character starts wearing, in slot order. */
		const std::vector<std::pair<std::string, LostArk::Shared::EQUIPMENT_SLOT>>&
			Get_StartingEquipment() const { return m_StartingEquipment; }

	private:
		std::unordered_map<std::string, SERVER_ITEM_DEFINITION> m_Items;
		std::vector<std::pair<std::string, LostArk::Shared::EQUIPMENT_SLOT>> m_StartingEquipment;
		std::string m_strStatus;
	};
}
