#include "ValtanClearRewards.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace
{
	// ItemCatalog와 같은 데이터 루트 선택 순서다. 환경변수가 없거나 버퍼에 맞지 않으면 실행 파일 기준 DataFiles를 사용한다.
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	// 행의 탭 경계를 나눠 원문을 차용하는 view를 반환한다. 호출자는 다음 getline으로 line을 바꾸기 전에 필요한 문자열을 복사한다.
	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(
				start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	// binary getline 뒤 남은 CR 한 글자만 제거하여 CRLF/LF 입력을 같은 행 내용으로 읽는다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	// from_chars가 입력 전체를 소비해야 성공이다. 실패 때 output이 원래 값이라는 보장은 없으므로 임시 parse 변수에 사용한다.
	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	// 게시 파일의 명시 직업 이름을 enum으로 바꾼다. 알 수 없는 이름이면 output을 바꾸지 않고 false다.
	bool ParseClass(const std::string_view value,
		LostArk::Shared::CHARACTER_CLASS_ID& output)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if ("LANCE_MASTER" == value)
			output = CHARACTER_CLASS_ID::LANCE_MASTER;
		else if ("GUNSLINGER" == value)
			output = CHARACTER_CLASS_ID::GUNSLINGER;
		else if ("SLAYER" == value)
			output = CHARACTER_CLASS_ID::SLAYER;
		else if ("ARTIST" == value)
			output = CHARACTER_CLASS_ID::ARTIST;
		else if ("DIMENSIONMASTER" == value)
			output = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		else if ("WARLORD" == value)
			output = CHARACTER_CLASS_ID::WARLORD;
		else if ("GUARDIANKNIGHT" == value)
			output = CHARACTER_CLASS_ID::GUARDIANKNIGHT;
		else
			return false;
		return true;
	}

	// 1~64바이트 ID에서 alnum 또는 밑줄/하이픈/점만 허용한다. 아이템 정의의 존재 여부를 조회하는 함수는 아니다.
	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 64u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}
}

// GameRoom 생성이 보상 목록을 준비한다. 기존 목록을 지역 변수에 옮겨 두고 각 false 경로에서 복구한다.
// 일반 실패 반환의 보존 경로이며, 문자열/vector 할당 예외를 별도로 잡아 복원하는 guard는 없다.
bool LostArk::Server::CValtanClearRewards::Load()
{
	CLASS_ITEM_IDS previousItemIds = std::move(m_ItemIds);
	m_ItemIds = {};

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Valtan" / L"ClearRewards.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing Valtan clear rewards bootstrap: " + path.string();
		m_ItemIds = std::move(previousItemIds);
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Valtan clear rewards bootstrap is empty";
		m_ItemIds = std::move(previousItemIds);
		return false;
	}
	StripCarriageReturn(line);
	// version/행 수를 먼저 제한한다. version 2만 읽으며 이전 형식을 추측해서 섞지 않는다.
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() ||
		"LOSTARK_VALTAN_CLEAR_REWARDS_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 512u)
	{
		m_strStatus = "Valtan clear rewards bootstrap header is invalid: " + path.string();
		m_ItemIds = std::move(previousItemIds);
		return false;
	}

	if (2u != version)
	{
		m_strStatus = "Valtan clear rewards bootstrap version mismatch: expected 2, got " +
			std::to_string(version) + "; path=" + path.string() +
			"; run powershell -ExecutionPolicy Bypass -File "
			"Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1 -Mode Publish";
		m_ItemIds = std::move(previousItemIds);
		return false;
	}

	// 게시 순서대로 해당 직업의 목록에 추가한다. 같은 item ID의 중복 행을 합치거나 제거하지 않는다.
	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Valtan clear rewards bootstrap row is truncated";
			m_ItemIds = std::move(previousItemIds);
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		LostArk::Shared::CHARACTER_CLASS_ID characterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		if (2u != fields.size() || !ParseClass(fields[0], characterClass) ||
			!IsStableId(fields[1]))
		{
			m_strStatus = "Valtan clear rewards bootstrap row is invalid";
			m_ItemIds = std::move(previousItemIds);
			return false;
		}
		m_ItemIds[static_cast<std::size_t>(characterClass)].emplace_back(fields[1]);
	}

	// 선언한 행 수를 모두 읽은 뒤 추가 행이 하나라도 있으면 기존 목록으로 복구한다.
	if (std::getline(input, line))
	{
		m_strStatus = "Valtan clear rewards bootstrap has trailing rows";
		m_ItemIds = std::move(previousItemIds);
		return false;
	}

	m_strStatus = "Loaded Valtan clear rewards bootstrap";
	return true;
}

// 알려진 직업만 배열에 접근한다. 목록이 없는 알려진 직업과 알 수 없는 직업 모두 빈 목록으로 순회할 수 있다.
const std::vector<std::string>& LostArk::Server::CValtanClearRewards::Get_ItemIds(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	static const std::vector<std::string> NONE;
	return LostArk::Shared::Is_Known_Character_Class(characterClass) ?
		m_ItemIds[static_cast<std::size_t>(characterClass)] : NONE;
}
