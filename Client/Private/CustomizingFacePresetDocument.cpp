#include "CustomizingFacePresetDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <filesystem>
#include <fstream>
#include <sstream>

bool_t Client::CCustomizingFacePresetDocument::Load()
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(L"UI/Customizing/CustomizingFacePresets.json");
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
		pSchema->Get_String() != "lostark.customizing-face-presets" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pClasses || !pClasses->Is_Object())
	{
		m_strStatus = path.string() + ": schema, formatVersion or classes is wrong";
		return false;
	}

	/* Staged, so a bad entry late in the document cannot leave half of the previous contents
	replaced. */
	std::unordered_map<std::string, std::vector<CUSTOMIZING_FACE_PRESET>> staged;
	for (const auto& [assetId, value] : pClasses->Get_Object())
	{
		const DATA_JSON_VALUE* pPresets = value.Is_Object() ? value.Find("preset") : nullptr;
		if (assetId.empty() || nullptr == pPresets || !pPresets->Is_Array() ||
			pPresets->Get_Array().empty())
		{
			m_strStatus = path.string() + " (" + assetId + "): preset is missing or empty";
			return false;
		}

		std::vector<CUSTOMIZING_FACE_PRESET> presets;
		presets.reserve(pPresets->Get_Array().size());
		for (const DATA_JSON_VALUE& entry : pPresets->Get_Array())
		{
			const DATA_JSON_VALUE* pIndex = entry.Is_Object() ? entry.Find("index") : nullptr;
			const DATA_JSON_VALUE* pSourceId =
				entry.Is_Object() ? entry.Find("sourcePresetId") : nullptr;
			const DATA_JSON_VALUE* pMorphs = entry.Is_Object() ? entry.Find("morphs") : nullptr;
			if (nullptr == pIndex || !pIndex->Is_Number() ||
				nullptr == pSourceId || !pSourceId->Is_String() ||
				nullptr == pMorphs || !pMorphs->Is_Array())
			{
				m_strStatus = path.string() + " (" + assetId +
					"): a preset is missing its index, sourcePresetId or morphs";
				return false;
			}
			/* The array position is the icon row position, so a document whose order drifted
			from its own indices would silently mis-pair thumbnails with faces. */
			if (pIndex->Get_Number() != static_cast<f64_t>(presets.size()))
			{
				m_strStatus = path.string() + " (" + assetId +
					"): array order does not match the presets' own index";
				return false;
			}

			CUSTOMIZING_FACE_PRESET preset;
			preset.strSourcePresetId = pSourceId->Get_String();
			preset.Morphs.reserve(pMorphs->Get_Array().size());
			for (const DATA_JSON_VALUE& morph : pMorphs->Get_Array())
			{
				const DATA_JSON_VALUE* pName = morph.Is_Object() ? morph.Find("name") : nullptr;
				const DATA_JSON_VALUE* pWeight =
					morph.Is_Object() ? morph.Find("weight") : nullptr;
				if (nullptr == pName || !pName->Is_String() || pName->Get_String().empty() ||
					nullptr == pWeight || !pWeight->Is_Number())
				{
					m_strStatus = path.string() + " (" + assetId + " / " +
						preset.strSourcePresetId + "): a morph is missing its name or weight";
					return false;
				}
				preset.Morphs.emplace_back(pName->Get_String(),
					static_cast<f32_t>(pWeight->Get_Number()));
			}
			presets.push_back(std::move(preset));
		}
		staged.emplace(assetId, std::move(presets));
	}

	m_Classes = std::move(staged);
	m_strStatus = "loaded " + std::to_string(m_Classes.size()) + " classes";
	return true;
}

const std::vector<Client::CUSTOMIZING_FACE_PRESET>*
	Client::CCustomizingFacePresetDocument::Find(const std::string& strAssetId) const
{
	const auto found = m_Classes.find(strAssetId);
	return found == m_Classes.end() ? nullptr : &found->second;
}
