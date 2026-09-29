#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Client
{
	/* One row of Data/Items/ItemCatalog.json. Debug-only slice: the F1 Give
	Item dropdown needs a display name, which the Server bootstrap does not
	carry, so the Client reads the source document directly the same way
	CPlayerSkillCatalog reads PlayerSkills.json. */
	struct ITEM_DEFINITION
	{
		std::string strItemId;
		std::string strDisplayName;
		std::uint32_t iMaxStack = 0;
		// Resources-relative path, or empty for an item with no icon art yet.
		std::string strIconPath;
		// 0 for a non-consumable; otherwise the percent of maximum HP a single
		// use restores. Display/UI-only -- the Server's own copy is what's
		// actually authoritative when a use is applied.
		std::uint32_t iHealPercent = 0;
		// "combat" (equipment, shown under the InventoryView Combat filter and
		// the item-upgrade window) or "use" (consumables/materials/currency,
		// shown under the Use filter). Display/filter-only, never sent to Server.
		std::string strCategory;
		/* Optional Client-only equipment presentation (absent = not equipment): the character
		info window slot this item fills ("weapon", "helmet", "shoulder", "top", "pants", "gloves",
		"necklace", "earring", "ring", "stone", "bracelet"), the class it belongs to ("Warlord",
		empty = any) and the grade background it is drawn on ("legend", "relic", ...). */
		std::string strEquipSlot;
		std::string strCharacterClass;
		std::string strGrade;
		/* Avatar only: the EquipmentPresentationCatalog visual set the character wears when
		   this item sits in its avatar slot. Empty for gear and for a shop template. */
		std::string strVisualSetId;
		/* Shop template only: characterClass -> the class item the Server hands the buyer.
		   Shown in the shop under the template's own name and icon; never equippable itself. */
		std::unordered_map<std::string, std::string> ClassVariants;
	};

	/* One stock line of an NPC shop (ItemCatalog.json "shops"). The Server's copy, published
	into Items.bootstrap, is what prices a purchase; this one only fills the shop window. */
	struct SHOP_ITEM_DEFINITION
	{
		std::string strItemId;
		/* "SILVER" or "GOLD" -- a currencies entry, not an item. */
		std::string strCurrencyId;
		std::uint32_t iPrice = 0;
	};

	/* One ItemCatalog.json "currencies" entry: the player's purse, shown under the inventory. */
	struct CURRENCY_DEFINITION
	{
		std::string strCurrencyId;
		std::string strDisplayName;
		std::string strIconPath;
	};

	struct SHOP_DEFINITION
	{
		std::string strShopId;
		std::vector<std::string> NpcPlacementIds;
		std::vector<SHOP_ITEM_DEFINITION> Items;
	};

	class CItemCatalog final
	{
	public:
		/* On failure the previously loaded set is kept and outStatus explains
		why, same contract as CPlayerSkillCatalog::Load. */
		static bool Load(std::string& outStatus);

		static const std::vector<ITEM_DEFINITION>& Get_Items();

		static const ITEM_DEFINITION* Find_ById(const std::string& itemId);

		/* The shop the NPC placement runs, or null. */
		static const SHOP_DEFINITION* Find_ShopByNpc(const std::string& npcPlacementId);
		static const std::vector<SHOP_DEFINITION>& Get_Shops();
		static const CURRENCY_DEFINITION* Find_Currency(const std::string& currencyId);
	};
}
