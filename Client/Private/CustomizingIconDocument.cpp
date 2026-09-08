#include "CustomizingIconDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
	constexpr const char_t* SCHEMA = "lostark.customizing-icons";
	constexpr const char_t* DOCUMENT = "UI/Customizing/CustomizingIcons.json";
	/* One class cannot need more cells than the retail table has rows for the
	biggest list; anything past this is a corrupt document rather than content. */
	constexpr size_t MAXIMUM_ICONS_PER_LIST = 512;

	bool_t Read_AssetList(
		const Client::DATA_JSON_VALUE* pValue,
		std::vector<std::string>& outList,
		std::string& outError)
	{
		outList.clear();
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array() || pValue->Get_Array().size() > MAXIMUM_ICONS_PER_LIST)
		{
			outError = "icon list is not an array of a sane length";
			return false;
		}
		for (const Client::DATA_JSON_VALUE& entry : pValue->Get_Array())
		{
			if (!entry.Is_String())
			{
				outError = "icon list holds a non-string entry";
				return false;
			}
			const std::string& asset = entry.Get_String();
			/* Resources-relative only: no absolute path and no escape upwards, the
			same rule every other runtime asset id follows. */
			if (asset.empty() || asset.find("..") != std::string::npos ||
				asset.find(':') != std::string::npos ||
				asset.front() == '/' || asset.front() == '\\')
			{
				outError = "icon asset id is not Resources-relative: " + asset;
				return false;
			}
			outList.push_back(asset);
		}
		return true;
	}
}

bool_t Client::CCustomizingIconDocument::Load()
{
	const std::filesystem::path path = CProjectDataRoot::Resolve(DOCUMENT);
	std::ifstream stream(path, std::ios::binary);
	if (!stream.is_open())
	{
		m_strStatus = "cannot open " + path.string();
		return false;
	}
	std::stringstream buffer;
	buffer << stream.rdbuf();

	DATA_JSON_VALUE root;
	std::string error;
	if (!CDataJson::Parse(buffer.str(), root, error) || !root.Is_Object())
	{
		m_strStatus = path.string() + ": " + error;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = root.Find("schema");
	const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
	const DATA_JSON_VALUE* pClasses = root.Find("classes");
	if (nullptr == pSchema || !pSchema->Is_String() || pSchema->Get_String() != SCHEMA ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pClasses || !pClasses->Is_Object())
	{
		m_strStatus = path.string() + ": schema, formatVersion or classes is wrong";
		return false;
	}

	/* Staged, so a bad entry late in the document cannot leave half of the previous
	contents replaced. */
	std::unordered_map<std::string, CLASS_ICONS> staged;
	for (const auto& [assetId, value] : pClasses->Get_Object())
	{
		if (assetId.empty() || !value.Is_Object())
		{
			m_strStatus = path.string() + ": class entry is not an object";
			return false;
		}
		CLASS_ICONS icons;
		if (!Read_AssetList(value.Find("preset"), icons.Presets, error) ||
			!Read_AssetList(value.Find("category1"), icons.FaceShapes, error) ||
			!Read_AssetList(value.Find("costume"), icons.Costumes, error) ||
			!Read_AssetList(value.Find("action"), icons.Actions, error) ||
			!Read_AssetList(value.Find("background"), icons.Backgrounds, error) ||
			!Read_AssetList(value.Find("category4"), icons.HairShapes, error) ||
			!Read_AssetList(value.Find("category9"), icons.EyeIrises, error) ||
			!Read_AssetList(value.Find("category10"), icons.AdornEyeLine, error) ||
			!Read_AssetList(value.Find("category11"), icons.AdornTouch, error) ||
			!Read_AssetList(value.Find("category12"), icons.AdornLip, error))
		{
			m_strStatus = path.string() + " (" + assetId + "): " + error;
			return false;
		}
		staged.emplace(assetId, std::move(icons));
	}

	m_Classes = std::move(staged);
	m_strStatus = "loaded " + std::to_string(m_Classes.size()) + " classes";
	return true;
}

const Client::CCustomizingIconDocument::CLASS_ICONS*
Client::CCustomizingIconDocument::Find(const std::string& strAssetId) const
{
	const auto found = m_Classes.find(strAssetId);
	return found == m_Classes.end() ? nullptr : &found->second;
}
