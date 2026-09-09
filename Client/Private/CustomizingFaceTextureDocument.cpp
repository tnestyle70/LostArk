#include "CustomizingFaceTextureDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <filesystem>
#include <fstream>
#include <sstream>

bool_t Client::CCustomizingFaceTextureDocument::Load()
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(L"UI/Customizing/CustomizingFaceTextures.json");
	std::ifstream stream(path);
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
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.customizing-face-textures" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pClasses || !pClasses->Is_Object())
	{
		m_strStatus = path.string() + ": schema, formatVersion or classes is wrong";
		return false;
	}

	/* Staged, so a bad entry late in the document cannot leave half of the previous contents
	replaced. */
	std::unordered_map<std::string,
		std::unordered_map<std::string, std::vector<std::string>>> staged;
	for (const auto& [assetId, classValue] : pClasses->Get_Object())
	{
		if (assetId.empty() || !classValue.Is_Object())
		{
			m_strStatus = path.string() + " (" + assetId + "): class entry is not an object";
			return false;
		}
		std::unordered_map<std::string, std::vector<std::string>> categories;
		for (const auto& [category, listValue] : classValue.Get_Object())
		{
			if (category.empty() || !listValue.Is_Array())
			{
				m_strStatus = path.string() + " (" + assetId + " / " + category +
					"): category is not an array";
				return false;
			}
			std::vector<std::string> textures;
			textures.reserve(listValue.Get_Array().size());
			for (const DATA_JSON_VALUE& entry : listValue.Get_Array())
			{
				const DATA_JSON_VALUE* pIndex =
					entry.Is_Object() ? entry.Find("index") : nullptr;
				if (nullptr == pIndex || !pIndex->Is_Number())
				{
					m_strStatus = path.string() + " (" + assetId + " / " + category +
						"): an entry is missing its index";
					return false;
				}
				/* A null textureAssetId is the cooker's hole marker: the retail list has an
				entry there but this client ships no texture for it. That is a gap in the art,
				not a broken document, so the position is kept and left empty -- rejecting the
				file over one took every other class's textures down with it. */
				const DATA_JSON_VALUE* pTexture = entry.Find("textureAssetId");
				const bool_t hasTexture = nullptr != pTexture &&
					pTexture->Is_String() && !pTexture->Get_String().empty();
				/* The cooker leaves a hole where the retail list names a texture this client
				does not ship, so index is the icon position and is *not* the array position.
				Padding keeps a click on icon N reaching texture N instead of sliding. */
				const size_t iIndex = static_cast<size_t>(pIndex->Get_Number());
				if (iIndex >= textures.size())
					textures.resize(iIndex + 1u);
				if (hasTexture)
					textures[iIndex] = pTexture->Get_String();
			}
			categories.emplace(category, std::move(textures));
		}
		staged.emplace(assetId, std::move(categories));
	}

	m_Classes = std::move(staged);
	m_strStatus = "loaded " + std::to_string(m_Classes.size()) + " classes";
	return true;
}

const std::vector<std::string>* Client::CCustomizingFaceTextureDocument::Find(
	const std::string& strAssetId, const std::string& strCategory) const
{
	const auto foundClass = m_Classes.find(strAssetId);
	if (m_Classes.end() == foundClass)
		return nullptr;
	const auto foundCategory = foundClass->second.find(strCategory);
	return foundCategory == foundClass->second.end() ? nullptr : &foundCategory->second;
}
