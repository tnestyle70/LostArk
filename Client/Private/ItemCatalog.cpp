#include "ItemCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <fstream>

namespace
{
	std::vector<Client::ITEM_DEFINITION> g_Items;
	std::vector<Client::SHOP_DEFINITION> g_Shops;
	std::vector<Client::CURRENCY_DEFINITION> g_Currencies;

	bool ReadDocument(
		const std::filesystem::path& relativePath,
		DATA_JSON_VALUE& output)
	{
		const std::filesystem::path path = CProjectDataRoot::Resolve(relativePath);
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
			return false;
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		std::string error;
		return CDataJson::Parse(text, output, error) && output.Is_Object();
	}

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool HasFormatVersion(
		const DATA_JSON_VALUE& document,
		const double expected)
	{
		const DATA_JSON_VALUE* version = document.Find("formatVersion");
		return nullptr != version &&
			version->Get_Type() == DATA_JSON_TYPE::NUMBER &&
			version->Get_Number() == expected;
	}
}

bool Client::CItemCatalog::Load(std::string& outStatus)
{
	DATA_JSON_VALUE root;
	if (!ReadDocument(L"Items/ItemCatalog.json", root))
	{
		outStatus = "Missing item catalog document";
		return false;
	}
	if (!HasFormatVersion(root, 2.0))
	{
		outStatus = "Item catalog document is not formatVersion 2";
		return false;
	}

	const DATA_JSON_VALUE* items = Required(root, "items", DATA_JSON_TYPE::ARRAY);
	if (nullptr == items)
	{
		outStatus = "ItemCatalog.json has no items array";
		return false;
	}

	/* Staged into a local first: a document that fails halfway must leave the
	catalog on its previous contents rather than half of the new ones. */
	std::vector<ITEM_DEFINITION> staged;
	for (const DATA_JSON_VALUE& value : items->Get_Array())
	{
		const DATA_JSON_VALUE* id = Required(value, "itemId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* name = Required(
			value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* maxStack = Required(
			value, "maxStack", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* iconPath = Required(
			value, "iconPath", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* healPercent = Required(
			value, "healPercent", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* category = Required(
			value, "category", DATA_JSON_TYPE::STRING);
		if (nullptr == id || id->Get_String().empty() ||
			nullptr == name || name->Get_String().empty() ||
			nullptr == maxStack || maxStack->Get_Number() < 1.0 ||
			nullptr == iconPath || nullptr == healPercent ||
			healPercent->Get_Number() < 0.0 || healPercent->Get_Number() > 100.0 ||
			nullptr == category ||
			(category->Get_String() != "combat" && category->Get_String() != "use"))
		{
			outStatus = "ItemCatalog.json has an invalid item";
			return false;
		}

		ITEM_DEFINITION definition{};
		definition.strItemId = id->Get_String();
		definition.strDisplayName = name->Get_String();
		definition.iMaxStack = static_cast<std::uint32_t>(maxStack->Get_Number());
		definition.strIconPath = iconPath->Get_String();
		definition.iHealPercent = static_cast<std::uint32_t>(healPercent->Get_Number());
		definition.strCategory = category->Get_String();
		const auto ReadOptionalText = [&value](const char* pKey, std::string& outText)
		{
			const DATA_JSON_VALUE* pText = value.Find(pKey);
			if (nullptr != pText && pText->Get_Type() == DATA_JSON_TYPE::STRING)
				outText = pText->Get_String();
		};
		ReadOptionalText("equipSlot", definition.strEquipSlot);
		ReadOptionalText("characterClass", definition.strCharacterClass);
		ReadOptionalText("grade", definition.strGrade);
		ReadOptionalText("visualSetId", definition.strVisualSetId);
		if (const DATA_JSON_VALUE* pVariants = value.Find("classVariants");
			nullptr != pVariants && pVariants->Get_Type() == DATA_JSON_TYPE::OBJECT)
		{
			for (const auto& [strClass, variant] : pVariants->Get_Object())
			{
				if (variant.Get_Type() != DATA_JSON_TYPE::STRING || variant.Get_String().empty())
				{
					outStatus = "ItemCatalog.json has an invalid classVariants entry";
					return false;
				}
				definition.ClassVariants[strClass] = variant.Get_String();
			}
		}

		for (const ITEM_DEFINITION& existing : staged)
		{
			if (existing.strItemId == definition.strItemId)
			{
				outStatus = "ItemCatalog.json repeats an item id";
				return false;
			}
		}
		staged.push_back(std::move(definition));
	}

	/* Optional currencies and shops. Same all-or-nothing rule as the items: a bad entry keeps
	every list on its previous contents. */
	std::vector<CURRENCY_DEFINITION> stagedCurrencies;
	if (const DATA_JSON_VALUE* currencies = root.Find("currencies"))
	{
		if (currencies->Get_Type() != DATA_JSON_TYPE::ARRAY)
		{
			outStatus = "ItemCatalog.json currencies is not an array";
			return false;
		}
		for (const DATA_JSON_VALUE& value : currencies->Get_Array())
		{
			const DATA_JSON_VALUE* currencyId = Required(value, "currencyId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* iconPath = Required(value, "iconPath", DATA_JSON_TYPE::STRING);
			if (nullptr == currencyId || nullptr == name || nullptr == iconPath)
			{
				outStatus = "ItemCatalog.json has an invalid currency";
				return false;
			}
			stagedCurrencies.push_back({ currencyId->Get_String(), name->Get_String(),
				iconPath->Get_String() });
		}
	}

	std::vector<SHOP_DEFINITION> stagedShops;
	if (const DATA_JSON_VALUE* shops = root.Find("shops"))
	{
		if (shops->Get_Type() != DATA_JSON_TYPE::ARRAY)
		{
			outStatus = "ItemCatalog.json shops is not an array";
			return false;
		}
		for (const DATA_JSON_VALUE& value : shops->Get_Array())
		{
			const DATA_JSON_VALUE* shopId = Required(value, "shopId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* npcIds = Required(value, "npcPlacementIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* stock = Required(value, "items", DATA_JSON_TYPE::ARRAY);
			if (nullptr == shopId || nullptr == npcIds || nullptr == stock)
			{
				outStatus = "ItemCatalog.json has an invalid shop";
				return false;
			}
			SHOP_DEFINITION shop{};
			shop.strShopId = shopId->Get_String();
			for (const DATA_JSON_VALUE& npcId : npcIds->Get_Array())
			{
				if (npcId.Get_Type() != DATA_JSON_TYPE::STRING)
				{
					outStatus = "ItemCatalog.json has an invalid shop NPC";
					return false;
				}
				shop.NpcPlacementIds.push_back(npcId.Get_String());
			}
			for (const DATA_JSON_VALUE& line : stock->Get_Array())
			{
				const DATA_JSON_VALUE* itemId = Required(line, "itemId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* currency = Required(line, "currencyId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* price = Required(line, "price", DATA_JSON_TYPE::NUMBER);
				if (nullptr == itemId || nullptr == currency || nullptr == price ||
					price->Get_Number() < 1.0)
				{
					outStatus = "ItemCatalog.json has an invalid shop item";
					return false;
				}
				shop.Items.push_back({ itemId->Get_String(), currency->Get_String(),
					static_cast<std::uint32_t>(price->Get_Number()) });
			}
			stagedShops.push_back(std::move(shop));
		}
	}

	g_Items = std::move(staged);
	g_Currencies = std::move(stagedCurrencies);
	g_Shops = std::move(stagedShops);
	outStatus = "Loaded " + std::to_string(g_Items.size()) + " items";
	return true;
}

const Client::SHOP_DEFINITION* Client::CItemCatalog::Find_ShopByNpc(
	const std::string& npcPlacementId)
{
	for (const SHOP_DEFINITION& shop : g_Shops)
		for (const std::string& npcId : shop.NpcPlacementIds)
			if (npcId == npcPlacementId)
				return &shop;
	return nullptr;
}

const std::vector<Client::SHOP_DEFINITION>& Client::CItemCatalog::Get_Shops()
{
	return g_Shops;
}

const Client::CURRENCY_DEFINITION* Client::CItemCatalog::Find_Currency(
	const std::string& currencyId)
{
	for (const CURRENCY_DEFINITION& currency : g_Currencies)
		if (currency.strCurrencyId == currencyId)
			return &currency;
	return nullptr;
}

const std::vector<Client::ITEM_DEFINITION>& Client::CItemCatalog::Get_Items()
{
	return g_Items;
}

const Client::ITEM_DEFINITION* Client::CItemCatalog::Find_ById(
	const std::string& itemId)
{
	for (const ITEM_DEFINITION& definition : g_Items)
	{
		if (definition.strItemId == itemId)
			return &definition;
	}
	return nullptr;
}
