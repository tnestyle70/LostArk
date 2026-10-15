// 탈것/스킬 정의의 게시 파일 로드와 룸 조회 본문을 구현한다.
#include "VehicleCatalog.h"

// 환경 데이터 루트와 실행 파일 경로를 UTF-16 API로 읽는다.
#include <Windows.h>

// 한 탈것 안에서 스킬 ID의 중복을 std::find로 검사한다.
#include <algorithm>
// ID·행 수·시간·이동 실수를 from_chars로 파싱한다.
#include <charconv>
// 속도·비행 profile·root motion 좌표의 NaN/무한대를 거부한다.
#include <cmath>
// 데이터 루트와 Vehicles/Vehicles.bootstrap 경로를 조합한다.
#include <filesystem>
// 게시된 tab 구분 파일을 binary getline으로 읽는다.
#include <fstream>
// 현재 줄과 root motion 토큰의 부분 문자열을 비소유 view로 검사한다.
#include <string_view>
// 필드 view 목록과 중복 검사할 스킬 ID 목록을 임시로 보관한다.
#include <vector>

namespace
{
	// 길이가 버퍼 안에 들어오는 LOSTARK_SERVER_DATA_ROOT를 우선하고 경로 존재 여부는 검사하지 않는다.
	// 그 외에는 실행 파일 디렉터리의 부모 아래 DataFiles를 사용하며 실행 파일 경로 조회 실패는 빈 경로다.
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

	// 현재 line을 차용하는 tab 필드 목록이다. 연속/끝 tab의 빈 필드도 남기고 line의 다음 getline 전에만 사용한다.
	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	// binary getline이 남긴 마지막 CR 하나만 없앤다. 필드 공백과 tab은 그대로 둔다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	// 정수/실수 출력에 직접 파싱하고 전체 문자열을 소비했을 때만 true다.
	// 숫자 prefix 뒤 문자가 남는 false에서도 output이 바뀔 수 있으므로 검증용 임시 값에만 사용한다.
	template <typename T> bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec && result.ptr == value.data() + value.size();
	}

	// 정확한 대문자 SPACE/Q/W/E만 enum으로 바꾼다. 다른 값은 output을 바꾸지 않고 false다.
	bool ParseSlot(const std::string_view value, LostArk::Server::VEHICLE_SKILL_SLOT& output)
	{
		using LostArk::Server::VEHICLE_SKILL_SLOT;
		if ("SPACE" == value)
			output = VEHICLE_SKILL_SLOT::SPACE;
		else if ("Q" == value)
			output = VEHICLE_SKILL_SLOT::Q;
		else if ("W" == value)
			output = VEHICLE_SKILL_SLOT::W;
		else if ("E" == value)
			output = VEHICLE_SKILL_SLOT::E;
		else
			return false;
		return true;
	}

	// VEHICLESKILL의 "timeMs:forward:lateral:up" 토큰을 쉼표로 나눈다. 시간은 엄격히 증가하고 동작 길이 이하여야 한다.
	// 표본이 0이면 "-"만 허용하고, 표본이 있으면 2~512개여야 한다. 시작 0ms/끝 limitMs 일치는 강제하지 않는다.
	// outSamples를 먼저 비우며 중간 실패에는 읽은 prefix가 남을 수 있다. Load는 지역 skill만 넘겨 실패 시 함께 폐기한다.
	bool ParseRootMotion(const std::string_view packed, const std::uint32_t sampleCount, const std::uint32_t limitMs,
	                     std::vector<LostArk::Server::ROOT_MOTION_SAMPLE>& outSamples)
	{
		outSamples.clear();
		if (0u == sampleCount)
			return "-" == packed;
		if (sampleCount < 2u || sampleCount > 512u)
			return false;
		// 표본 개수와 실제 문자열 끝이 함께 맞아야 한다. 덜 읽은 표본·초과 토큰·끝 쉼표를 성공으로 허용하지 않는다.
		std::size_t cursor = 0u;
		bool reachedEnd = false;
		while (!reachedEnd && outSamples.size() < sampleCount)
		{
			const std::size_t comma = packed.find(',', cursor);
			const std::string_view token =
			    packed.substr(cursor, std::string_view::npos == comma ? std::string_view::npos : comma - cursor);
			const std::size_t first = token.find(':');
			const std::size_t second = std::string_view::npos == first ? first : token.find(':', first + 1u);
			const std::size_t third = std::string_view::npos == second ? second : token.find(':', second + 1u);
			LostArk::Server::ROOT_MOTION_SAMPLE sample{};
			if (std::string_view::npos == third || std::string_view::npos != token.find(':', third + 1u) ||
			    !ParseNumber(token.substr(0u, first), sample.iTimeMs) ||
			    !ParseNumber(token.substr(first + 1u, second - first - 1u), sample.fForward) ||
			    !ParseNumber(token.substr(second + 1u, third - second - 1u), sample.fLateral) ||
			    !ParseNumber(token.substr(third + 1u), sample.fUp) || !std::isfinite(sample.fForward) ||
			    !std::isfinite(sample.fLateral) || !std::isfinite(sample.fUp) || sample.iTimeMs > limitMs ||
			    (!outSamples.empty() && sample.iTimeMs <= outSamples.back().iTimeMs))
			{
				return false;
			}
			outSamples.push_back(sample);
			reachedEnd = std::string_view::npos == comma;
			if (!reachedEnd)
				cursor = comma + 1u;
		}
		return reachedEnd && outSamples.size() == sampleCount;
	}
}

// GameRoom 초기화가 Publish-VehicleProfiles.ps1의 Vehicles.bootstrap을 읽는다. 성공 전의 정의는 지역 staged만 소유한다.
// 명시적인 false는 기존 m_Vehicles를 보존한다. 일부 profile 거절은 이전 진단이 남으며 할당/파일시스템 예외를 여기서 catch하지 않는다.
bool LostArk::Server::CVehicleCatalog::Load()
{
	// 기존 catalog와 분리된 후보다. 뒤의 최종 map 교체 전 반환하면 지역 수명으로 폐기된다.
	using VEHICLE_MAP = decltype(m_Vehicles);
	VEHICLE_MAP staged;

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Vehicles" / L"Vehicles.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing vehicle bootstrap: " + path.string();
		return false;
	}

	// header는 magic·version 2/3/4·전체 행 수 1~4096이다. 행 수에는 VEHICLE뿐 아니라 바로 뒤의 VEHICLESKILL도 포함한다.
	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Vehicle bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0u;
	std::uint32_t rowCount = 0u;
	if (3u != header.size() || "LOSTARK_VEHICLE_BOOTSTRAP" != header[0] || !ParseNumber(header[1], version) ||
	    (2u != version && 3u != version && 4u != version) || !ParseNumber(header[2], rowCount) || 0u == rowCount ||
	    rowCount > 4096u)
	{
		m_strStatus = "Vehicle bootstrap header is invalid";
		return false;
	}

	// 탈것 행을 읽고 그 행이 선언한 스킬 수만큼 다음 행을 즉시 소비한다. 스킬 행을 독립 순서로 배치할 수 없다.
	for (std::uint32_t row = 0u; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Vehicle bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		// version 2는 기본 4필드, 3은 flight를 포함한 10필드, 4는 boost까지 11필드다. 없는 선택 값은 구조체의 0 기본값이다.
		SERVER_VEHICLE_DEFINITION vehicle{};
		std::uint32_t skillCount = 0u;
		if ((version >= 4u ? 11u : (version == 3u ? 10u : 4u)) != fields.size() || "VEHICLE" != fields[0] ||
		    !ParseNumber(fields[1], vehicle.iVehicleId) || LostArk::Shared::INVALID_VEHICLE_ID == vehicle.iVehicleId ||
		    !ParseNumber(fields[2], vehicle.fMoveSpeed) || !std::isfinite(vehicle.fMoveSpeed) ||
		    vehicle.fMoveSpeed <= 0.f || vehicle.fMoveSpeed > 30.f || !ParseNumber(fields[3], skillCount) ||
		    skillCount > 4u || row + skillCount >= rowCount)
		{
			m_strStatus = "Vehicle bootstrap row is invalid";
			return false;
		}
		// flight 여섯 수치를 finite 0~60 범위로 읽는다. 양수 이륙 시간은 Ancient Sea만 허용하며 다른 값의 관계도 검사한다.
		// 비행을 끈 행은 나머지 값도 모두 0이어야 한다. 이 블록의 false는 m_strStatus를 새로 설정하지 않는다.
		if (version >= 3u)
		{
			float* flight[] = {&vehicle.fFlightTakeoffSeconds, &vehicle.fFlightLandingSeconds,
			                   &vehicle.fFlightHoverHeight,    &vehicle.fFlightMaximumHeight,
			                   &vehicle.fFlightSpeed,          &vehicle.fFlightVerticalSpeed};
			for (std::size_t index = 0u; index < std::size(flight); ++index)
				if (!ParseNumber(fields[index + 4u], *flight[index]) || !std::isfinite(*flight[index]) ||
				    *flight[index] < 0.f || *flight[index] > 60.f)
					return false;
			if (vehicle.Has_Flight() && (vehicle.iVehicleId != LostArk::Shared::ANCIENT_SEA_VEHICLE_ID ||
			                             vehicle.fFlightLandingSeconds <= 0.f || vehicle.fFlightHoverHeight < .1f ||
			                             vehicle.fFlightMaximumHeight < vehicle.fFlightHoverHeight ||
			                             vehicle.fFlightSpeed <= 0.f || vehicle.fFlightVerticalSpeed <= 0.f ||
			                             vehicle.fFlightMaximumHeight / vehicle.fFlightVerticalSpeed > 600.f))
				return false;
			if (!vehicle.Has_Flight())
				for (const auto field : flight)
					if (*field != 0.f)
						return false;
		}
		if (version >= 4u)
		{
			// boost는 일반 속도보다 빠른 0~30m/s의 유한 값이며 비행과 함께 켤 수 없다. 0이면 boost 없음이다.
			// 이 검증의 false도 이전 진단 문자열을 그대로 남긴다.
			if (!ParseNumber(fields[10], vehicle.fBoostMoveSpeed) || !std::isfinite(vehicle.fBoostMoveSpeed) ||
			    vehicle.fBoostMoveSpeed < 0.f || vehicle.fBoostMoveSpeed > 30.f)
				return false;
			if (vehicle.Has_Boost() && (vehicle.fBoostMoveSpeed <= vehicle.fMoveSpeed || vehicle.Has_Flight()))
				return false;
		}
		// 중복 검사는 현재 탈것 안에서만 수행한다. 입력 슬롯은 bit mask, 스킬 ID는 지역 목록으로 추적한다.
		std::uint8_t usedSlots = 0u;
		std::vector<LostArk::Shared::SKILL_ID> skillIds;
		for (std::uint32_t skillRow = 0u; skillRow < skillCount; ++skillRow)
		{
			if (!std::getline(input, line))
			{
				m_strStatus = "Vehicle bootstrap row is truncated";
				return false;
			}
			// 바깥 header 행 수에서도 이 스킬 행을 소비한 것으로 기록한다. 앞선 vehicle fields는 line 교체 뒤 다시 사용하지 않는다.
			++row;
			StripCarriageReturn(line);
			const std::vector<std::string_view> skillFields = SplitTabs(line);
			SERVER_VEHICLE_SKILL skill{};
			LostArk::Shared::VEHICLE_ID ownerId = LostArk::Shared::INVALID_VEHICLE_ID;
			std::uint32_t sampleCount = 0u;
			// 소유 탈것 ID·유효 스킬 ID·입력 슬롯·시간 범위·root motion·중복을 함께 검사한다.
			// cooldown 0은 허용하지만 동작 길이는 1~60000ms이며 root motion 표본 수는 ParseRootMotion이 확인한다.
			if (8u != skillFields.size() || "VEHICLESKILL" != skillFields[0] || !ParseNumber(skillFields[1], ownerId) ||
			    ownerId != vehicle.iVehicleId || !ParseNumber(skillFields[2], skill.iSkillId) ||
			    LostArk::Shared::INVALID_SKILL_ID == skill.iSkillId || !ParseSlot(skillFields[3], skill.eSlot) ||
			    !ParseNumber(skillFields[4], skill.iCooldownMs) || skill.iCooldownMs > 600000u ||
			    !ParseNumber(skillFields[5], skill.iActionDurationMs) || 0u == skill.iActionDurationMs ||
			    skill.iActionDurationMs > 60000u || !ParseNumber(skillFields[6], sampleCount) ||
			    !ParseRootMotion(skillFields[7], sampleCount, skill.iActionDurationMs, skill.RootMotion) ||
			    0u != (usedSlots & (1u << static_cast<std::uint8_t>(skill.eSlot))) ||
			    std::find(skillIds.begin(), skillIds.end(), skill.iSkillId) != skillIds.end())
			{
				m_strStatus = "Vehicle bootstrap skill row is invalid";
				return false;
			}
			usedSlots |= static_cast<std::uint8_t>(1u << static_cast<std::uint8_t>(skill.eSlot));
			skillIds.push_back(skill.iSkillId);
			vehicle.Skills.push_back(std::move(skill));
		}
		// 완성된 탈것을 후보 map에 옮긴다. 같은 ID의 두 번째 정의는 덮어쓰지 않고 거부한다.
		if (!staged.emplace(vehicle.iVehicleId, std::move(vehicle)).second)
		{
			m_strStatus = "Duplicate vehicle ID";
			return false;
		}
	}

	// 선언한 전체 행 뒤의 추가 줄은 빈 줄도 거부한다. EOF와 I/O 실패를 따로 분류하지는 않는다.
	if (std::getline(input, line))
	{
		m_strStatus = "Vehicle bootstrap has trailing rows";
		return false;
	}

	// 모든 명시적 검증을 통과한 후보를 교체한다. 기존 정의를 차용하던 포인터는 성공한 교체 뒤 재사용하면 안 된다.
	m_Vehicles = std::move(staged);
	m_strStatus = "Loaded vehicle bootstrap";
	return true;
}

// 룸이 탈것의 권위 이동/스킬 수치를 읽는다. 미등록은 nullptr이며 상태나 플레이어를 변경하지 않는다.
const LostArk::Server::SERVER_VEHICLE_DEFINITION* LostArk::Server::CVehicleCatalog::Find_Vehicle(
    const LostArk::Shared::VEHICLE_ID vehicleId) const
{
	const auto iter = m_Vehicles.find(vehicleId);
	return m_Vehicles.end() == iter ? nullptr : &iter->second;
}
