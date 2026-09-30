#include "ItemCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <fstream>
#include <cmath>
#include <limits>

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

	bool ReadPreviewTint(const DATA_JSON_VALUE* value, float4_t& output)
	{
		if (!value || !value->Is_Array() || value->Get_Array().size() != 4u)
			return false;
		float components[4]{};
		for (size_t index = 0u; index < 4u; ++index)
		{
			const auto& entry = value->Get_Array()[index];
			if (!entry.Is_Number() || !std::isfinite(entry.Get_Number()) ||
				entry.Get_Number() < 0. || entry.Get_Number() > 100. ||
				(index == 3u && entry.Get_Number() > 1.))
				return false;
			components[index] = static_cast<float>(entry.Get_Number());
		}
		output = { components[0], components[1], components[2], components[3] };
		return true;
	}

	bool ReadGroundPreview(const DATA_JSON_VALUE* value,
		Client::PLAYER_SKILL_TARGET_PREVIEW& output)
	{
		if (!value || !value->Is_Object() || value->Get_Object().size() != 8u)
			return false;
		const auto* asset = Required(*value, "textureAssetId", DATA_JSON_TYPE::STRING);
		const auto* coverage = Required(*value, "coverageChannel", DATA_JSON_TYPE::STRING);
		const auto* diameterFraction = Required(*value, "textureDiameterFraction", DATA_JSON_TYPE::NUMBER);
		const auto* identity = Required(*value, "assetIdentityBasis", DATA_JSON_TYPE::STRING);
		const auto* usage = Required(*value, "usageBasis", DATA_JSON_TYPE::STRING);
		const auto* evidence = Required(*value, "sourceEvidence", DATA_JSON_TYPE::STRING);
		if (!asset || !coverage || !identity || !usage || !evidence ||
			!diameterFraction || !std::isfinite(diameterFraction->Get_Number()) ||
			diameterFraction->Get_Number() <= 0. || diameterFraction->Get_Number() > 1. ||
			coverage->Get_String() != "R" ||
			identity->Get_String() != "SOURCE_VERIFIED" ||
			usage->Get_String() != "PROJECT_COMPOSITION" || evidence->Get_String().empty() ||
			!ReadPreviewTint(value->Find("validTint"), output.vValidTint) ||
			!ReadPreviewTint(value->Find("invalidTint"), output.vInvalidTint))
			return false;
		const auto& path = asset->Get_String();
		if (path.rfind("Effect/", 0u) != 0u || path.size() > 260u ||
			path.size() < 4u || path.substr(path.size() - 4u) != ".dds" ||
			path.find('\\') != std::string::npos || path.find(':') != std::string::npos)
			return false;
		for (const auto& component : std::filesystem::path(path))
			if (component == ".." || component == ".")
				return false;
		output.fDiameter = static_cast<float>(1. / diameterFraction->Get_Number());
		if (!std::isfinite(output.fDiameter))
			return false;
		output.strAssetId = path;
		output.strAssetIdentityBasis = identity->Get_String();
		output.strUsageBasis = usage->Get_String();
		output.strSourceEvidence = evidence->Get_String();
		return true;
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
		if (const auto* battle = value.Find("battleUse"))
		{
			const auto* range = Required(*battle, "rangeCm", DATA_JSON_TYPE::NUMBER);
			if (!range || !std::isfinite(range->Get_Number()) ||
				range->Get_Number() < 0. || range->Get_Number() > 5000.)
			{ outStatus = "ItemCatalog.json has an invalid battle target range"; return false; }
			const auto* skillId = Required(*battle, "skillId", DATA_JSON_TYPE::NUMBER);
			const auto* cooldownMs = Required(*battle, "cooldownMs", DATA_JSON_TYPE::NUMBER);
			const auto IsUnsignedInteger = [](const DATA_JSON_VALUE* number)
			{
				return number && std::isfinite(number->Get_Number()) && number->Get_Number() >= 0. &&
					number->Get_Number() <= (std::numeric_limits<std::uint32_t>::max)() &&
					std::floor(number->Get_Number()) == number->Get_Number();
			};
			if (!IsUnsignedInteger(skillId) || skillId->Get_Number() == 0. || !IsUnsignedInteger(cooldownMs))
			{ outStatus = "ItemCatalog.json has an invalid battle cooldown"; return false; }
			definition.fTargetRangeM = static_cast<float>(range->Get_Number() * .01);
			definition.iBattleSkillId = static_cast<std::uint32_t>(skillId->Get_Number());
			definition.iCooldownMs = static_cast<std::uint32_t>(cooldownMs->Get_Number());
			const auto* kind = Required(*battle, "kind", DATA_JSON_TYPE::STRING);
			if (!kind)
			{ outStatus = "ItemCatalog.json has an invalid battle kind"; return false; }
			definition.isGroundTargeted = kind->Get_String() == "DESTRUCTION" ||
				kind->Get_String() == "WHIRLWIND";
			if (definition.isGroundTargeted)
			{
				const auto* radius = Required(*battle, "radiusCm", DATA_JSON_TYPE::NUMBER);
				const auto* preview = Required(value, "groundTargetPreview", DATA_JSON_TYPE::OBJECT);
				if (!radius || !std::isfinite(radius->Get_Number()) ||
					radius->Get_Number() <= 0. || radius->Get_Number() > 5000. ||
					definition.fTargetRangeM <= 0.f || !preview ||
					preview->Get_Object().size() != 2u ||
					!ReadGroundPreview(preview->Find("rangePreview"), definition.RangePreview) ||
					!ReadGroundPreview(preview->Find("targetPreview"), definition.TargetPreview))
				{ outStatus = "ItemCatalog.json has an invalid ground target preview"; return false; }
				definition.RangePreview.fDiameter *= 2.f * definition.fTargetRangeM;
				definition.TargetPreview.fDiameter *= static_cast<float>(radius->Get_Number() * .02);
				if (!std::isfinite(definition.RangePreview.fDiameter) || !std::isfinite(definition.TargetPreview.fDiameter))
				{ outStatus = "ItemCatalog.json has an invalid ground preview extent"; return false; }
			}

		}
		if (value.Find("groundTargetPreview") && !definition.isGroundTargeted)
		{ outStatus = "ItemCatalog.json assigns a ground preview to a non-thrown item"; return false; }
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
