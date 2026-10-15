// 칭호 목록 로드와 membership 조회를 선언한 catalog의 본문을 구현한다.
#include "HonorTitleCatalog.h"

// Server 데이터 루트 환경 변수와 현재 실행 파일 경로를 UTF-16 Win32 API로 읽는다.
#include <Windows.h>

// 버전·행 수·칭호 ID를 locale 없이 읽고 숫자 뒤 잔여 문자를 검사한다.
#include <charconv>
// 데이터 루트와 게시 bootstrap 상대 경로를 조합한다.
#include <filesystem>
// bootstrap을 binary 입력 stream으로 열고 줄 단위로 검사한다.
#include <fstream>
// 한 줄의 tab 구간을 복사 없이 나누어 header/행 필드를 파싱한다.
#include <string_view>
// 한 줄을 가리키는 필드 view 목록을 임시로 보관한다.
#include <vector>

namespace
{
	// Load가 게시 파일의 기준 경로를 얻는다. 길이 범위 안의 LOSTARK_SERVER_DATA_ROOT를 우선하고
	// 그 외에는 실행 파일 디렉터리의 부모 아래 DataFiles를 선택한다.
	// 환경 값의 존재/폴더 유효성을 여기서 검사하지 않으며 실행 파일 경로 조회가 실패하면 빈 경로를 반환한다.
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength =
		    GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", configured, static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() / L"DataFiles";
	}

	// binary getline이 남긴 마지막 CR 하나만 제거한다. 공백/tab이나 필드 내용을 trim하지 않는다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	// 원본 line을 가리키는 비소유 view들을 만든다. 연속 tab·끝 tab의 빈 필드도 남겨 형식 검증에 사용한다.
	// 반환 view는 line이 다시 읽히거나 변경되기 전에만 소비해야 한다.
	std::vector<std::string_view> SplitTabs(const std::string_view line)
	{
		std::vector<std::string_view> fields;
		std::size_t start = 0u;
		while (true)
		{
			const std::size_t tab = line.find('\t', start);
			if (std::string_view::npos == tab)
			{
				fields.push_back(line.substr(start));
				return fields;
			}
			fields.push_back(line.substr(start, tab - start));
			start = tab + 1u;
		}
	}

	// 호출자가 준비한 정수 출력에 from_chars로 직접 파싱하고 문자열 전체를 소비했을 때만 true다.
	// 숫자 prefix 뒤 문자가 남으면 false여도 output은 바뀔 수 있으므로 Load의 임시 변수에만 사용한다.
	template <typename T> bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec && result.ptr == value.data() + value.size();
	}
}

// GameRoom 초기화의 칭호 허용 목록 로드다. 모든 행을 지역 staged에 모아 기존 목록을 중간에 노출하지 않는다.
// 명시적인 false 경로는 m_Titles를 보존하고 m_strStatus를 바꾸며, 이 함수는 할당/파일시스템 예외를 catch하지 않는다.
bool LostArk::Server::CHonorTitleCatalog::Load()
{
	// 실제 catalog와 분리된 후보 집합이다. 뒤의 최종 교체 전 실패하면 지역 수명으로 폐기한다.
	std::unordered_set<LostArk::Shared::HONOR_TITLE_ID> staged;

	// 파일 경로를 먼저 조합해 열기 실패의 진단에도 동일한 경로를 남긴다.
	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"HonorTitles" / L"HonorTitles.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing honor title bootstrap: " + path.string();
		return false;
	}

	// 첫 줄은 magic·schema version 1·행 수(1~4096)의 세 필드여야 한다.
	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Honor title bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0u;
	std::uint32_t rowCount = 0u;
	if (3u != header.size() || "LOSTARK_HONOR_TITLE_BOOTSTRAP" != header[0] || !ParseNumber(header[1], version) ||
	    1u != version || !ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Honor title bootstrap header is invalid";
		return false;
	}

	// 선언한 수만큼 TITLE/유효 ID 두 필드를 읽고 중복 ID를 거부한다.
	// fields는 아래 검증/삽입을 마칠 때까지만 현재 line을 참조한다.
	for (std::uint32_t row = 0u; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Honor title bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		LostArk::Shared::HONOR_TITLE_ID titleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;
		if (2u != fields.size() || "TITLE" != fields[0] || !ParseNumber(fields[1], titleId) ||
		    LostArk::Shared::INVALID_HONOR_TITLE_ID == titleId)
		{
			m_strStatus = "Honor title bootstrap row is invalid";
			return false;
		}
		if (!staged.insert(titleId).second)
		{
			m_strStatus = "Duplicate honor title ID";
			return false;
		}
	}

	// 선언된 행 다음에 한 줄이라도 더 읽히면 빈 줄도 trailing row로 거부한다.
	if (std::getline(input, line))
	{
		m_strStatus = "Honor title bootstrap has trailing rows";
		return false;
	}

	// 모든 명시적 검증을 통과한 뒤 목록을 한 번 교체하고 성공 진단을 남긴다.
	m_Titles = std::move(staged);
	m_strStatus = "Loaded honor title bootstrap";
	return true;
}

// 룸의 장착/복원/전환 검증이 부르는 읽기 전용 조회다. ID 해석이나 플레이어 상태 변경은 하지 않는다.
bool LostArk::Server::CHonorTitleCatalog::Has_Title(const LostArk::Shared::HONOR_TITLE_ID titleId) const
{
	return m_Titles.contains(titleId);
}
