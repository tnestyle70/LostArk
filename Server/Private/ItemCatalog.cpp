#include "ItemCatalog.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace
{
	// Same resolution rule as CGameplayCatalog::Resolve_DataRoot: an explicit
	// override first, otherwise the DataFiles folder next to the running exe.
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(
				start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	/* The character info window's slot names; "-" is no starting slot. */
	bool ParseStartingSlot(const std::string_view value,
		LostArk::Shared::EQUIPMENT_SLOT& output)
	{
		using LostArk::Shared::EQUIPMENT_SLOT;
		static constexpr std::pair<std::string_view, EQUIPMENT_SLOT> SLOTS[] = {
			{ "-", EQUIPMENT_SLOT::NONE }, { "helmet", EQUIPMENT_SLOT::HELMET },
			{ "shoulder", EQUIPMENT_SLOT::SHOULDER }, { "top", EQUIPMENT_SLOT::TOP },
			{ "pants", EQUIPMENT_SLOT::PANTS }, { "gloves", EQUIPMENT_SLOT::GLOVES },
			{ "weapon", EQUIPMENT_SLOT::WEAPON }, { "necklace", EQUIPMENT_SLOT::NECKLACE },
			{ "earring1", EQUIPMENT_SLOT::EARRING1 }, { "earring2", EQUIPMENT_SLOT::EARRING2 },
			{ "ring1", EQUIPMENT_SLOT::RING1 }, { "ring2", EQUIPMENT_SLOT::RING2 },
			{ "stone", EQUIPMENT_SLOT::STONE }, { "bracelet", EQUIPMENT_SLOT::BRACELET },
		};
		for (const auto& [name, slot] : SLOTS)
		{
			if (name == value)
			{
				output = slot;
				return true;
			}
		}
		return false;
	}

	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 64u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}
}

bool LostArk::Server::CItemCatalog::Load()
{
	using ITEM_MAP = decltype(m_Items);
	ITEM_MAP previousItems = std::move(m_Items);
	m_Items.clear();
	std::unordered_map<std::string, std::string> shopIdByNpc;
	std::unordered_map<std::string, std::vector<SERVER_SHOP_ITEM>> shopItems;
	/* (template, class) -> variant, applied to m_Items after every ITEM row is known. */
	std::vector<std::array<std::string, 3>> variantRows;
	SERVER_PURSE startingPurse{};
	const auto parseCurrency = [](const std::string_view value, SERVER_CURRENCY& output)
	{
		if ("SILVER" == value) { output = SERVER_CURRENCY::SILVER; return true; }
		if ("GOLD" == value) { output = SERVER_CURRENCY::GOLD; return true; }
		return false;
	};

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Items" / L"Items.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing item bootstrap: " + path.string();
		m_Items = std::move(previousItems);
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Item bootstrap is empty";
		m_Items = std::move(previousItems);
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() || "LOSTARK_ITEM_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Item bootstrap header is invalid: " + path.string();
		m_Items = std::move(previousItems);
		return false;
	}

	if (7u != version)
	{
		m_strStatus = "Item bootstrap version mismatch: expected 7, got " +
			std::to_string(version) + "; path=" + path.string() +
			"; run powershell -ExecutionPolicy Bypass -File "
			"Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Publish";
		m_Items = std::move(previousItems);
		return false;
	}

	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Item bootstrap row is truncated";
			m_Items = std::move(previousItems);
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		/* CURRENCY <SILVER|GOLD> <startingAmount>: the fresh-character purse. */
		if (3u == fields.size() && "CURRENCY" == fields[0])
		{
			SERVER_CURRENCY currency{};
			std::uint32_t amount = 0;
			if (!parseCurrency(fields[1], currency) || !ParseNumber(fields[2], amount))
			{
				m_strStatus = "Item bootstrap CURRENCY row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			startingPurse.Amount(currency) = amount;
			continue;
		}
		/* SHOPNPC <shopId> <npcPlacementId>: the NPC runs that shop. */
		if (3u == fields.size() && "SHOPNPC" == fields[0])
		{
			if (!IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!shopIdByNpc.emplace(std::string(fields[2]), std::string(fields[1])).second)
			{
				m_strStatus = "Item bootstrap SHOPNPC row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			continue;
		}
		/* SHOPITEM <shopId> <itemId> <SILVER|GOLD> <price>: one stock line. */
		if (5u == fields.size() && "SHOPITEM" == fields[0])
		{
			SERVER_SHOP_ITEM stock{};
			if (!IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!parseCurrency(fields[3], stock.eCurrency) ||
				!ParseNumber(fields[4], stock.iPrice) || 0u == stock.iPrice)
			{
				m_strStatus = "Item bootstrap SHOPITEM row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			stock.strItemId = fields[2];
			shopItems[std::string(fields[1])].push_back(std::move(stock));
			continue;
		}
		if (15u == fields.size() && "BATTLEITEM" == fields[0])
		{
			auto item = m_Items.find(std::string(fields[1]));
			SERVER_BATTLE_ITEM_USE use;
			if (fields[2] == "DESTRUCTION" && fields[1] == "BATTLE_DESTRUCTION_BOMB") use.eKind = BATTLE_ITEM_KIND::DESTRUCTION;
			if (fields[2] == "WHIRLWIND" && fields[1] == "BATTLE_WHIRLWIND_GRENADE") use.eKind = BATTLE_ITEM_KIND::WHIRLWIND;
			if (fields[2] == "CLEANSE" && fields[1] == "BATTLE_HOLY_CHARM") use.eKind = BATTLE_ITEM_KIND::CLEANSE;
			if (fields[2] == "TIME_STOP" && fields[1] == "BATTLE_TIME_STOP_POTION") use.eKind = BATTLE_ITEM_KIND::TIME_STOP;
			if (item == m_Items.end() || use.eKind == BATTLE_ITEM_KIND::NONE ||
				item->second.BattleUse.eKind != BATTLE_ITEM_KIND::NONE || item->second.iHealPercent || !item->second.strEquipSlot.empty() ||
				!ParseNumber(fields[3], use.iSkillId) || !use.iSkillId ||
				!ParseNumber(fields[4], use.iDamageRatePercent) || use.iDamageRatePercent > 1000000u ||
				!ParseNumber(fields[5], use.iPartDamage) || use.iPartDamage > 1000000u ||
				!ParseNumber(fields[6], use.iStaggerDamage) || use.iStaggerDamage > 1000000u ||
				!ParseNumber(fields[7], use.iRangeCm) || use.iRangeCm > 5000u ||
				!ParseNumber(fields[8], use.iRadiusCm) || use.iRadiusCm > 5000u ||
				!ParseNumber(fields[9], use.iDurationMs) || use.iDurationMs > 600000u ||
				!ParseNumber(fields[10], use.iCooldownMs) || use.iCooldownMs < 1000u || use.iCooldownMs > 600000u ||
				((use.eKind == BATTLE_ITEM_KIND::DESTRUCTION || use.eKind == BATTLE_ITEM_KIND::WHIRLWIND) && (!use.iRangeCm || !use.iRadiusCm)) ||
				(use.eKind == BATTLE_ITEM_KIND::TIME_STOP && use.iRangeCm) ||
				((use.eKind == BATTLE_ITEM_KIND::CLEANSE || use.eKind == BATTLE_ITEM_KIND::TIME_STOP) && !use.iDurationMs) ||
				!ParseNumber(fields[11], use.iProjectileSpeedCmPerSecond) || use.iProjectileSpeedCmPerSecond > 10000u ||
				!ParseNumber(fields[12], use.iProjectileArcHeightCm) || use.iProjectileArcHeightCm > 1000u ||
				!ParseNumber(fields[13], use.iProjectileLaunchHeightCm) || use.iProjectileLaunchHeightCm > 1000u ||
				!ParseNumber(fields[14], use.iStaggerMaximumDivisor) ||
				(use.eKind == BATTLE_ITEM_KIND::WHIRLWIND ? (use.iStaggerMaximumDivisor != 3u || use.iDamageRatePercent || use.iStaggerDamage) : use.iStaggerMaximumDivisor != 0u) ||
				((use.eKind == BATTLE_ITEM_KIND::DESTRUCTION || use.eKind == BATTLE_ITEM_KIND::WHIRLWIND) && !use.iProjectileSpeedCmPerSecond))
			{
				m_strStatus = "Item bootstrap BATTLEITEM row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			item->second.BattleUse = use;
			continue;
		}
		/* ITEMVARIANT <templateId> <className> <itemId>: the class variant a template buys. */
		if (4u == fields.size() && "ITEMVARIANT" == fields[0])
		{
			if (!IsStableId(fields[1]) || fields[2].empty() || !IsStableId(fields[3]))
			{
				m_strStatus = "Item bootstrap ITEMVARIANT row is invalid";
				m_Items = std::move(previousItems);
				return false;
			}
			variantRows.push_back({ std::string(fields[1]), std::string(fields[2]), std::string(fields[3]) });
			continue;
		}
		SERVER_ITEM_DEFINITION item{};
		if (7u != fields.size() || "ITEM" != fields[0] || !IsStableId(fields[1]) ||
			!ParseNumber(fields[2], item.iMaxStack) || 0u == item.iMaxStack ||
			!ParseNumber(fields[3], item.iHealPercent) || item.iHealPercent > 100u ||
			fields[4].empty() || fields[5].empty() ||
			!ParseStartingSlot(fields[6], item.eStartingEquippedSlot))
		{
			m_strStatus = "Item bootstrap row is invalid";
			m_Items = std::move(previousItems);
			return false;
		}
		item.strItemId = fields[1];
		/* "-" is the publisher's "no value" marker for the two optional columns. */
		if ("-" != fields[4])
			item.strEquipSlot = fields[4];
		if ("-" != fields[5])
			item.strCharacterClass = fields[5];
		if (!m_Items.emplace(item.strItemId, std::move(item)).second)
		{
			m_strStatus = "Duplicate item ID";
			m_Items = std::move(previousItems);
			return false;
		}
	}

	if (std::getline(input, line))
	{
		m_strStatus = "Item bootstrap has trailing rows";
		m_Items = std::move(previousItems);
		return false;
	}

	/* A template is not equipment and every variant is equipment of exactly that class;
	   the publisher checks the same so a hand-edited file stays honest. */
	for (const auto& [templateId, className, variantId] : variantRows)
	{
		const auto templateIter = m_Items.find(templateId);
		const auto variantIter = m_Items.find(variantId);
		if (m_Items.end() == templateIter || m_Items.end() == variantIter ||
			!templateIter->second.strEquipSlot.empty() || !templateIter->second.strCharacterClass.empty() ||
			variantIter->second.strEquipSlot.empty() || variantIter->second.strCharacterClass != className ||
			!templateIter->second.ClassVariants.emplace(className, variantId).second)
		{
			m_strStatus = "Item bootstrap ITEMVARIANT row does not fit its items: " + templateId;
			m_Items = std::move(previousItems);
			return false;
		}
	}

	/* Every stock line must name a catalog item, and every shop an NPC runs must sell
	   something. The publisher checks the same; this keeps a hand-edited file honest. */
	for (const auto& [shopId, stock] : shopItems)
		for (const SERVER_SHOP_ITEM& line : stock)
			if (!m_Items.contains(line.strItemId))
			{
				m_strStatus = "Item bootstrap shop sells an unknown item";
				m_Items = std::move(previousItems);
				return false;
			}
	for (const auto& [npcId, shopId] : shopIdByNpc)
		if (!shopItems.contains(shopId))
		{
			m_strStatus = "Item bootstrap shop has no stock: " + shopId;
			m_Items = std::move(previousItems);
			return false;
		}

	m_StartingEquipment.clear();
	for (const auto& [itemId, item] : m_Items)
		if (LostArk::Shared::EQUIPMENT_SLOT::NONE != item.eStartingEquippedSlot)
			m_StartingEquipment.emplace_back(itemId, item.eStartingEquippedSlot);
	std::sort(m_StartingEquipment.begin(), m_StartingEquipment.end(),
		[](const auto& left, const auto& right) { return left.second < right.second; });
	m_StartingPurse = startingPurse;
	m_ShopIdByNpcPlacementId = std::move(shopIdByNpc);
	m_ShopItemsByShopId = std::move(shopItems);
	m_strStatus = "Loaded item bootstrap";
	return true;
}

const LostArk::Server::SERVER_ITEM_DEFINITION*
LostArk::Server::CItemCatalog::Find_Item(const std::string& itemId) const
{
	const auto iter = m_Items.find(itemId);
	return m_Items.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::SERVER_SHOP_ITEM*
LostArk::Server::CItemCatalog::Find_ShopItem(
	const std::string& npcPlacementId, const std::string& itemId) const
{
	const auto shop = m_ShopIdByNpcPlacementId.find(npcPlacementId);
	if (m_ShopIdByNpcPlacementId.end() == shop)
		return nullptr;
	const auto stock = m_ShopItemsByShopId.find(shop->second);
	if (m_ShopItemsByShopId.end() == stock)
		return nullptr;
	for (const SERVER_SHOP_ITEM& line : stock->second)
		if (line.strItemId == itemId)
			return &line;
	return nullptr;
}
