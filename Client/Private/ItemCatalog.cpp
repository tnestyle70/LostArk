#include "ItemCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <fstream>

namespace
{
	std::vector<Client::ITEM_DEFINITION> g_Items;

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
		if (nullptr == id || id->Get_String().empty() ||
			nullptr == name || name->Get_String().empty() ||
			nullptr == maxStack || maxStack->Get_Number() < 1.0 ||
			nullptr == iconPath || nullptr == healPercent ||
			healPercent->Get_Number() < 0.0 || healPercent->Get_Number() > 100.0)
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

	g_Items = std::move(staged);
	outStatus = "Loaded " + std::to_string(g_Items.size()) + " items";
	return true;
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
