#pragma once

#include "Network/PacketMessages.h"
#include "ServerPlayer.h"

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
		/* ITEMVARIANT rows: a shop template's class name -> the item the buyer of that class
		   receives. Empty for an ordinary item. A template is never equipment itself. */
		std::unordered_map<std::string, std::string> ClassVariants;
	};

	// One stock line of an NPC shop: the item, the currency it is paid in and the price of one.
	struct SERVER_SHOP_ITEM
	{
		std::string strItemId;
		SERVER_CURRENCY eCurrency = SERVER_CURRENCY::SILVER;
		std::uint32_t iPrice = 0;
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
		/* The purse a fresh character starts with (CURRENCY rows). */
		const SERVER_PURSE& Get_StartingPurse() const { return m_StartingPurse; }

		/* The stock line for itemId in the shop npcPlacementId runs; null when that NPC
		   runs no shop or its shop does not sell the item. */
		const SERVER_SHOP_ITEM* Find_ShopItem(
			const std::string& npcPlacementId, const std::string& itemId) const;

	private:
		std::unordered_map<std::string, SERVER_ITEM_DEFINITION> m_Items;
		std::vector<std::pair<std::string, LostArk::Shared::EQUIPMENT_SLOT>> m_StartingEquipment;
		SERVER_PURSE m_StartingPurse;
		std::unordered_map<std::string, std::string> m_ShopIdByNpcPlacementId;
		std::unordered_map<std::string, std::vector<SERVER_SHOP_ITEM>> m_ShopItemsByShopId;
		std::string m_strStatus;
	};
}
