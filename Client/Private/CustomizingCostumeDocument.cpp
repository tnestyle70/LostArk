#include "CustomizingCostumeDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <filesystem>
#include <fstream>
#include <sstream>

Client::CCustomizingCostumeDocument::CCustomizingCostumeDocument(
	std::string strSchema, std::string strDocument, std::string strArrayName)
	: m_strSchema(std::move(strSchema))
	, m_strDocument(std::move(strDocument))
	, m_strArrayName(std::move(strArrayName))
{
}

bool_t Client::CCustomizingCostumeDocument::Load()
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(std::filesystem::path(m_strDocument).wstring());
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
	if (nullptr == pSchema || !pSchema->Is_String() || pSchema->Get_String() != m_strSchema ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pClasses || !pClasses->Is_Object())
	{
		m_strStatus = path.string() + ": schema, formatVersion or classes is wrong";
		return false;
	}

	/* Staged, so a bad entry late in the document cannot leave half of the previous contents
	replaced. */
	std::unordered_map<std::string, std::vector<std::string>> staged;
	for (const auto& [assetId, value] : pClasses->Get_Object())
	{
		const DATA_JSON_VALUE* pEntries =
			value.Is_Object() ? value.Find(m_strArrayName) : nullptr;
		if (assetId.empty() || nullptr == pEntries || !pEntries->Is_Array() ||
			pEntries->Get_Array().empty())
		{
			m_strStatus = path.string() + " (" + assetId + "): " + m_strArrayName +
				" is missing or empty";
			return false;
		}

		std::vector<std::string> setIds;
		setIds.reserve(pEntries->Get_Array().size());
		for (const DATA_JSON_VALUE& entry : pEntries->Get_Array())
		{
			/* Costumes call it objectUnit (the table's own column) and hairstyles call it
			index; either way it is the position the icon list pairs against. */
			const DATA_JSON_VALUE* pUnit =
				entry.Is_Object() ? entry.Find("objectUnit") : nullptr;
			if (nullptr == pUnit && entry.Is_Object())
				pUnit = entry.Find("index");
			const DATA_JSON_VALUE* pSetId =
				entry.Is_Object() ? entry.Find("visualSetId") : nullptr;
			if (nullptr == pUnit || !pUnit->Is_Number() ||
				nullptr == pSetId || !pSetId->Is_String() || pSetId->Get_String().empty())
			{
				m_strStatus = path.string() + " (" + assetId +
					"): an entry is missing its index or visualSetId";
				return false;
			}
			/* The array position is the Object_Unit the icon row uses, so a document whose
			order drifted from its own indices would silently mis-pair thumbnails. */
			if (pUnit->Get_Number() != static_cast<f64_t>(setIds.size()))
			{
				m_strStatus = path.string() + " (" + assetId +
					"): array order does not match the entries' own index";
				return false;
			}
			setIds.push_back(pSetId->Get_String());
		}
		staged.emplace(assetId, std::move(setIds));
	}

	m_Classes = std::move(staged);
	m_strStatus = "loaded " + std::to_string(m_Classes.size()) + " classes";
	return true;
}

const std::vector<std::string>* Client::CCustomizingCostumeDocument::Find(
	const std::string& strAssetId) const
{
	const auto found = m_Classes.find(strAssetId);
	return found == m_Classes.end() ? nullptr : &found->second;
}
