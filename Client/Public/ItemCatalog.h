#pragma once

#include <cstdint>
#include <string>
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
	};

	class CItemCatalog final
	{
	public:
		/* On failure the previously loaded set is kept and outStatus explains
		why, same contract as CPlayerSkillCatalog::Load. */
		static bool Load(std::string& outStatus);

		static const std::vector<ITEM_DEFINITION>& Get_Items();

		static const ITEM_DEFINITION* Find_ById(const std::string& itemId);
	};
}
