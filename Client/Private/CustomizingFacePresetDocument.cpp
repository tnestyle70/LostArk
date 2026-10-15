#include "CustomizingFacePresetDocument.h"

// JSON의 object/array/number/string을 검사하고 값으로 읽는다.
#include "DataJson.h"
// 저작 정본 Data 아래 문서 경로를 구한다.
#include "ProjectDataRoot.h"

// 문서 파일 경로를 조합·전달한다.
#include <filesystem>
// 문서 파일의 바이트를 입력 stream으로 읽는다.
#include <fstream>
// 파일 전체를 모아 JSON parser에 전달한다.
#include <sstream>

// CustomizingView가 얼굴 목록을 준비할 때 호출한다. morph 이름 존재 여부와 실제 모델 적용은 Character 쪽에서 처리한다.
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

	// 후반 preset 검사 실패가 이전 class 목록을 부분 변경하지 않게 임시 map에 모은다.
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
			// index가 현재 배열 위치와 같은지 확인해 아이콘과 얼굴 가중치의 대응이 밀리지 않게 한다.
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
				// 가중치는 number를 f32_t로 복사한다. 이 reader에서 추가 범위 제한이나 실제 morph 이름 조회는 하지 않는다.
				preset.Morphs.emplace_back(pName->Get_String(),
					static_cast<f32_t>(pWeight->Get_Number()));
			}
			presets.push_back(std::move(preset));
		}
		staged.emplace(assetId, std::move(presets));
	}

	// 모든 명시적 검사를 통과한 뒤 이전 목록을 교체한다. 이전 Find 결과 포인터는 다시 조회해야 한다.
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
