#include "CustomizingIconDocument.h"

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

namespace
{
	constexpr const char_t* SCHEMA = "lostark.customizing-icons";
	constexpr const char_t* DOCUMENT = "UI/Customizing/CustomizingIcons.json";
	// 한 목록은 최대 512개로 제한해 비정상적인 문서 크기를 거부한다.
	constexpr size_t MAXIMUM_ICONS_PER_LIST = 512;

	// 출력은 먼저 비운다. 생략한 key는 빈 목록으로 허용하고, 중간 실패의 임시 출력은 Load가 폐기한다.
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
			// 빈 값·.. 포함·colon·선행 slash를 거부해 Resources 상대 ID 형식을 제한한다. 파일 존재까지 확인하지는 않는다.
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

	// 모든 class 목록을 임시 map에 모아 뒤쪽 행 실패가 기존 목록을 부분 변경하지 않게 한다.
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

	// 모든 명시적 검사를 통과한 뒤 이전 목록을 교체한다. 이전 Find 결과 포인터는 다시 조회해야 한다.
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
