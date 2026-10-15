#include "CustomizingCostumeDocument.h"

// JSON의 object/array/number/string을 검사하고 값으로 읽는다.
#include "DataJson.h"
// 저작 정본 Data 아래 문서 경로를 구한다.
#include "ProjectDataRoot.h"

// 기본 visualSetId가 목록에 정확히 한 번 있는지 확인하고 그 위치를 찾는다.
#include <algorithm>
// 문서 파일 경로를 조합·전달한다.
#include <filesystem>
// 문서 파일의 바이트를 입력 stream으로 읽는다.
#include <fstream>
// 파일 전체를 모아 JSON parser에 전달한다.
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

	// 목록과 기본 선택을 함께 임시로 준비해 뒤쪽 행 실패 시 이전 내용을 유지한다.
	std::unordered_map<std::string, std::vector<std::string>> staged;
	std::unordered_map<std::string, int32_t> stagedDefaults;
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
			// 의상의 objectUnit을 먼저 읽고 없을 때 헤어의 index를 읽는다. 두 값 모두 아이콘 위치와 대응한다.
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
			// 배열 순서와 저장 index가 같은지 검사해 visualSetId와 아이콘의 대응이 밀리지 않게 한다.
			if (pUnit->Get_Number() != static_cast<f64_t>(setIds.size()))
			{
				m_strStatus = path.string() + " (" + assetId +
					"): array order does not match the entries' own index";
				return false;
			}
			setIds.push_back(pSetId->Get_String());
		}
		int32_t defaultIndex = 0;
		// body 기본 머리 선택은 hairstyle 문서만 허용하며 true일 때 별도 defaultVisualSetId와 함께 쓸 수 없다.
		if (const auto* bodyHair = value.Find("defaultBodyHair"))
		{
			if (m_strSchema != "lostark.customizing-hairstyles" ||
				m_strArrayName != "hairstyle" || !bodyHair->Is_Boolean())
			{
				m_strStatus = path.string() + " (" + assetId +
					"): defaultBodyHair must be a boolean in a hairstyle document";
				return false;
			}
			if (bodyHair->Get_Boolean())
			{
				if (nullptr != value.Find("defaultVisualSetId"))
				{
					m_strStatus = path.string() + " (" + assetId +
						"): defaultBodyHair and defaultVisualSetId cannot both select a default";
					return false;
				}
				defaultIndex = -1;
			}
		}
		// 기본 set ID는 현재 목록에 정확히 한 번 있어야 한다. 실제 catalog 모델 준비는 여기서 수행하지 않는다.
		if (const auto* defaultSet = value.Find("defaultVisualSetId"))
		{
			if (!defaultSet->Is_String() || defaultSet->Get_String().empty() ||
				std::count(setIds.begin(), setIds.end(), defaultSet->Get_String()) != 1)
			{
				m_strStatus = path.string() + " (" + assetId +
					"): defaultVisualSetId must name exactly one existing entry";
				return false;
			}
			defaultIndex = static_cast<int32_t>(std::distance(setIds.begin(),
				std::find(setIds.begin(), setIds.end(), defaultSet->Get_String())));
		}
		stagedDefaults.emplace(assetId, defaultIndex);
		staged.emplace(assetId, std::move(setIds));
	}

	// 모든 명시적 검사를 통과한 뒤 이전 목록을 교체한다. 이전 Find 결과 포인터는 다시 조회해야 한다.
	m_Classes = std::move(staged);
	m_DefaultIndices = std::move(stagedDefaults);
	m_strStatus = "loaded " + std::to_string(m_Classes.size()) + " classes";
	return true;
}

int32_t Client::CCustomizingCostumeDocument::Get_DefaultIndex(const std::string& strAssetId) const
{
	const auto found = m_DefaultIndices.find(strAssetId);
	return found == m_DefaultIndices.end() ? 0 : found->second;
}

const std::vector<std::string>* Client::CCustomizingCostumeDocument::Find(
	const std::string& strAssetId) const
{
	const auto found = m_Classes.find(strAssetId);
	return found == m_Classes.end() ? nullptr : &found->second;
}
