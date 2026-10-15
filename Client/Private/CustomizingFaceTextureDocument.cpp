#include "CustomizingFaceTextureDocument.h"

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

// CustomizingView가 목록을 준비할 때 호출한다. schema/version·행 구조를 확인하고 texture 파일 자체는 열지 않는다.
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

	// 행 하나가 잘못되어도 이전 class 목록이 부분 교체되지 않도록 임시 map에서 읽는다.
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
				// textureAssetId가 비어 있지 않은 문자열일 때만 해당 index에 기록한다. 아니면 같은 index의 기존 값도 지우지 않는다.
				const DATA_JSON_VALUE* pTexture = entry.Find("textureAssetId");
				const bool_t hasTexture = nullptr != pTexture &&
					pTexture->Is_String() && !pTexture->Get_String().empty();
				// index를 icon 위치로 사용해 빈 칸을 채운다. 배열 순서와 index의 일치·정수 범위를 이 코드가 별도로 검사하지는 않는다.
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

	// 모든 명시적 검사를 통과한 뒤 이전 목록을 교체한다. 이전 Find 결과 포인터는 다시 조회해야 한다.
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
