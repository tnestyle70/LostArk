// 게시 행을 profile/anchor/group/wave/entry의 소유 값으로 파싱한다.
#include "SpawnGroupBootstrap.h"

// 데이터 루트 환경변수와 Server EXE 경로를 조회한다.
#include <Windows.h>

// 그룹과 wave 안에 비어 있는 목록이 있는지 최종 검사한다.
#include <algorithm>
// 탭 필드 전체를 locale에 의존하지 않는 정수/실수로 해석한다.
#include <charconv>
// anchor와 이동/피격 수치의 유한성·절댓값 범위를 확인한다.
#include <cmath>
// world별 spawngroupsbootstrap 파일 경로를 구성한다.
#include <filesystem>
// 게시 파일을 binary로 열고 줄 단위로 읽는다.
#include <fstream>
// 현재 한 줄의 탭 필드를 복사 없이 차용한다.
#include <string_view>
// 현재 본문에는 unordered_set의 직접 사용이 없다. include 구성을 바꾸지 않고 기존 의존성을 유지한다.
#include <unordered_set>

namespace
{
	using namespace LostArk::Server;
	using LostArk::Shared::WORLD_ID;
	constexpr float MAX_ABSOLUTE_ANCHOR_COMPONENT = 100000.f;

	// 위치(m)와 yaw(degree)에 같은 수치 상한 100000을 적용한다. 단위 변환이나 navigation 유효성 검사는 수행하지 않는다.
	bool IsValidAnchorComponent(const float value)
	{
		return std::isfinite(value) && std::fabs(value) <= MAX_ABSOLUTE_ANCHOR_COMPONENT;
	}

	// 지원 world를 파일명/헤더의 고정 문자열로 연결한다. 반환 view는 문자열 리터럴 수명이며 미지원 ID는 빈 값이다.
	std::string_view World_ToString(const WORLD_ID worldId)
	{
		switch (worldId)
		{
		case WORLD_ID::BERN:
			return "BERN";
		case WORLD_ID::VALTAN_ARENA:
			return "VALTAN_ARENA";
		case WORLD_ID::TRAINING_GROUND:
			return "TRAINING_GROUND";
		case WORLD_ID::CHARACTER_SELECT_ARENA:
			return "CHARACTER_SELECT_ARENA";
		case WORLD_ID::KOUKUSAYDON_ARENA:
			return "KOUKUSAYDON_ARENA";
		case WORLD_ID::MAHARAKA:
			return "MAHARAKA";
		case WORLD_ID::COLOSSEUM:
			return "COLOSSEUM";
		default:
			return {};
		}
	}

	// 유효 길이의 LOSTARK_SERVER_DATA_ROOT를 우선하고 없거나 너무 길면 EXE 부모의 부모 아래 DataFiles를 사용한다.
	// EXE 조회 실패는 빈 루트로 반환되므로 호출부 조합 결과는 상대 경로가 될 수 있다.
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

	// 빈 필드와 끝의 빈 필드도 보존한다. 반환 view는 line을 차용하므로 다음 getline 전에 필요한 ID를 소유 문자열로 옮긴다.
	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> result;
		std::string_view view(line);
		size_t start = 0;
		while (true)
		{
			const size_t tab = view.find('\t', start);
			result.push_back(view.substr(start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return result;
	}

	// 필드 전체 소비까지 확인한다. 숫자 prefix를 읽은 뒤 나머지 문자로 false가 되면 outValue는 이미 바뀌었을 수 있어 지역 후보에만 사용한다.
	template <typename T> bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto parsed = std::from_chars(value.data(), value.data() + value.size(), outValue);
		return std::errc{} == parsed.ec && parsed.ptr == value.data() + value.size();
	}

	// binary getline이 남긴 끝 CR 한 글자만 제거한다. 필드 내부 공백이나 BOM을 임의로 정리하지 않는다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}
}

// 룸 초기화가 world별 선택 게시 파일을 읽는다. 파일을 연 뒤에는 지역 staging에 모든 정의를 구성하고 최종 검사를 통과해야 멤버를 교체한다.
// 파일 열기 실패의 빈 성공은 별도 정책이며 명시적 false는 정의와 revision을 보존하고 진단만 바꾼다. 예외 처리 경계는 두지 않는다.
bool LostArk::Server::CSpawnGroupBootstrap::Load(const LostArk::Shared::WORLD_ID worldId)
{
	const std::string_view worldName = World_ToString(worldId);
	if (worldName.empty())
	{
		m_strStatus = "Unknown spawn group world ID";
		return false;
	}
	const std::filesystem::path path =
	    Resolve_DataRoot() / L"World" / std::filesystem::path(std::string(worldName) + ".spawngroupsbootstrap");
	std::ifstream input(path, std::ios::binary);
	// 선택 spawn 문서로 취급하므로 열기 실패면 기존 목록도 비운다. 여기서는 파일 부재와 접근/기타 열기 오류를 구분하지 않는다.
	if (!input)
	{
		m_Groups.clear();
		m_Anchors.clear();
		m_Profiles.clear();
		m_iRevision = 0;
		m_strStatus = "No spawn groups for world";
		return true;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Empty spawn group bootstrap";
		return false;
	}
	StripCarriageReturn(line);
	// schema4의 world·revision·상위 행 개수를 검사한다. Area ID인 header[3]은 이 loader가 읽거나 대조하지 않는다.
	const auto header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t revision = 0;
	std::uint32_t anchorCount = 0;
	std::uint32_t groupCount = 0;
	std::uint32_t profileCount = 0;
	if (8u != header.size() || "LOSTARK_SPAWN_GROUP_BOOTSTRAP" != header[0] || !ParseNumber(header[1], version) ||
	    4u != version || header[2] != worldName || !ParseNumber(header[4], revision) || 0u == revision ||
	    !ParseNumber(header[5], anchorCount) || anchorCount > 128u || !ParseNumber(header[6], groupCount) ||
	    groupCount > 32u || !ParseNumber(header[7], profileCount) || profileCount > 128u)
	{
		m_strStatus = "Spawn group bootstrap header is invalid";
		return false;
	}

	// 반환 포인터를 공개하지 않는 지역 정의 목록이다. group/wave index는 뒤 행이 이미 읽은 부모를 찾는 데 사용한다.
	std::unordered_map<std::string, MONSTER_RUNTIME_PROFILE> stagedProfiles;
	std::unordered_map<std::string, SPAWN_GROUP_ANCHOR> stagedAnchors;
	std::vector<SPAWN_GROUP_DEFINITION> stagedGroups;
	std::unordered_map<std::string, size_t> groupIndices;
	std::unordered_map<std::string, std::pair<size_t, size_t>> waveIndices;
	while (std::getline(input, line))
	{
		StripCarriageReturn(line);
		if (line.empty())
			continue;
		const auto fields = SplitTabs(line);
		// PROFILE 23필드의 전투·이동·피격 수치와 서로 연관된 flag/시간을 검사한다. archetype ID의 중복은 거부하지만 전체 저작 ID 규칙 검사는 publisher가 맡는다.
		if (!fields.empty() && "PROFILE" == fields[0])
		{
			MONSTER_RUNTIME_PROFILE profile;
			std::uint32_t attackKnockdownFlag = 0u;
			if (23u != fields.size() || !ParseNumber(fields[2], profile.iMaxHp) || 0u == profile.iMaxHp ||
			    !ParseNumber(fields[3], profile.iAttackPower) || !ParseNumber(fields[4], profile.iDefense) ||
			    !ParseNumber(fields[5], profile.fCollisionRadius) || !ParseNumber(fields[6], profile.fEngageRange) ||
			    !ParseNumber(fields[7], profile.fMoveSpeed) || !ParseNumber(fields[8], profile.fAttackRange) ||
			    !ParseNumber(fields[9], profile.iAttackWindupMs) || !ParseNumber(fields[10], profile.iAttackActiveMs) ||
			    !ParseNumber(fields[11], profile.iAttackRecoveryMs) ||
			    !ParseNumber(fields[12], profile.iDeadDespawnMs) ||
			    !ParseNumber(fields[13], profile.fHitKnockbackScale) ||
			    !ParseNumber(fields[14], profile.fAttackPushRangeM) ||
			    !ParseNumber(fields[15], profile.iAttackPushMs) || !ParseNumber(fields[16], attackKnockdownFlag) ||
			    !ParseNumber(fields[17], profile.iAttackDownMs) ||
			    !ParseNumber(fields[18], profile.fTargetReleaseRange) ||
			    !ParseNumber(fields[19], profile.fTurnSpeedDegreesPerSecond) ||
			    !ParseNumber(fields[20], profile.fAcceleration) || !ParseNumber(fields[21], profile.fDeceleration) ||
			    !ParseNumber(fields[22], profile.fArrivalSlowRadius) || !std::isfinite(profile.fCollisionRadius) ||
			    profile.fCollisionRadius <= 0.f || !std::isfinite(profile.fEngageRange) ||
			    profile.fEngageRange <= 0.f || !std::isfinite(profile.fMoveSpeed) || profile.fMoveSpeed <= 0.f ||
			    !std::isfinite(profile.fAttackRange) || profile.fAttackRange <= 0.f ||
			    !std::isfinite(profile.fTargetReleaseRange) || profile.fTargetReleaseRange < profile.fEngageRange ||
			    !std::isfinite(profile.fTurnSpeedDegreesPerSecond) || profile.fTurnSpeedDegreesPerSecond <= 0.f ||
			    !std::isfinite(profile.fAcceleration) || profile.fAcceleration <= 0.f ||
			    !std::isfinite(profile.fDeceleration) || profile.fDeceleration <= 0.f ||
			    !std::isfinite(profile.fArrivalSlowRadius) || profile.fArrivalSlowRadius < profile.fCollisionRadius ||
			    !std::isfinite(profile.fHitKnockbackScale) || profile.fHitKnockbackScale < 0.f ||
			    !std::isfinite(profile.fAttackPushRangeM) || std::fabs(profile.fAttackPushRangeM) > 20.f ||
			    attackKnockdownFlag > 1u || (0.f != profile.fAttackPushRangeM && 0u == profile.iAttackPushMs) ||
			    (0.f == profile.fAttackPushRangeM && 0u != profile.iAttackPushMs) ||
			    (1u == attackKnockdownFlag && 0u == profile.iAttackDownMs) ||
			    (0u == attackKnockdownFlag && 0u != profile.iAttackDownMs))
			{
				m_strStatus = "Spawn group monster profile row is invalid";
				return false;
			}
			profile.bAttackKnockdown = 1u == attackKnockdownFlag;
			profile.strArchetypeId = fields[1];
			if (!stagedProfiles.emplace(profile.strArchetypeId, profile).second)
			{
				m_strStatus = "Duplicate spawn group monster profile";
				return false;
			}
		}
		// ANCHOR 위치와 yaw를 유한한 제한값으로 읽는다. 지면 투영과 실제 생성 가능성은 Spawn_Monster가 다시 확인한다.
		else if (!fields.empty() && "ANCHOR" == fields[0])
		{
			SPAWN_GROUP_ANCHOR anchor;
			if (6u != fields.size() || !ParseNumber(fields[2], anchor.fPositionX) ||
			    !ParseNumber(fields[3], anchor.fPositionY) || !ParseNumber(fields[4], anchor.fPositionZ) ||
			    !ParseNumber(fields[5], anchor.fYawDegrees) || !IsValidAnchorComponent(anchor.fPositionX) ||
			    !IsValidAnchorComponent(anchor.fPositionY) || !IsValidAnchorComponent(anchor.fPositionZ) ||
			    !IsValidAnchorComponent(anchor.fYawDegrees))
			{
				m_strStatus = "Spawn group anchor row is invalid";
				return false;
			}
			anchor.strAnchorId = fields[1];
			if (!stagedAnchors.emplace(anchor.strAnchorId, anchor).second)
			{
				m_strStatus = "Duplicate spawn group anchor";
				return false;
			}
		}
		// GROUP ID와 maxAlive·반복 정책을 등록한다. 선언 waveCount는 범위 검사와 reserve에 쓰이며 실제 wave 수와의 최종 일치 검사는 없다.
		else if (!fields.empty() && "GROUP" == fields[0])
		{
			SPAWN_GROUP_DEFINITION group;
			std::uint32_t waveCount = 0;
			if (7u != fields.size() || !ParseNumber(fields[3], group.iMaxAlive) || 0u == group.iMaxAlive ||
			    group.iMaxAlive > 64u || !ParseNumber(fields[4], waveCount) || 0u == waveCount || waveCount > 16u ||
			    !ParseNumber(fields[6], group.iRepeatDelayMs) || group.iRepeatDelayMs > 600000u)
			{
				m_strStatus = "Spawn group definition row is invalid";
				return false;
			}
			if ("ONCE" == fields[5])
				group.eRepeatPolicy = SPAWN_GROUP_REPEAT_POLICY::ONCE;
			else if ("REPEAT" == fields[5])
				group.eRepeatPolicy = SPAWN_GROUP_REPEAT_POLICY::REPEAT;
			else
			{
				m_strStatus = "Spawn group repeat policy is unknown";
				return false;
			}
			// REPEAT는 양수 지연, ONCE는 0이어야 한다. 해석하지 않을 지연 값을 조용히 무시하지 않는다.
			if ((SPAWN_GROUP_REPEAT_POLICY::REPEAT == group.eRepeatPolicy) != (0u != group.iRepeatDelayMs))
			{
				m_strStatus = "Spawn group repeat delay contradicts its policy";
				return false;
			}
			group.strSpawnGroupId = fields[1];
			group.strRequiredCompletedGroupId = "-" == fields[2] ? "" : std::string(fields[2]);
			if (!groupIndices.emplace(group.strSpawnGroupId, stagedGroups.size()).second)
			{
				m_strStatus = "Duplicate spawn group definition";
				return false;
			}
			group.Waves.reserve(waveCount);
			stagedGroups.push_back(std::move(group));
		}
		// 먼저 등록된 GROUP에 0부터 연속하는 wave index로 붙인다. entryCount는 범위 검사/reserve용이며 실제 entry 수와 같다는 검사는 하지 않는다.
		else if (!fields.empty() && "WAVE" == fields[0])
		{
			std::uint32_t waveIndex = 0;
			std::uint32_t entryCount = 0;
			SPAWN_GROUP_WAVE wave;
			const auto group = 3u <= fields.size() ? groupIndices.find(std::string(fields[1])) : groupIndices.end();
			if (8u != fields.size() || groupIndices.end() == group || !ParseNumber(fields[3], waveIndex) ||
			    waveIndex != stagedGroups[group->second].Waves.size() || !ParseNumber(fields[4], wave.iStartDelayMs) ||
			    !ParseNumber(fields[5], entryCount) || 0u == entryCount || entryCount > 16u ||
			    !ParseNumber(fields[7], wave.iNextWaveDelayMs) || wave.iNextWaveDelayMs > 600000u)
			{
				m_strStatus = "Spawn group wave row is invalid";
				return false;
			}
			if ("ALL_DEAD" == fields[6])
				wave.eNextWavePolicy = SPAWN_NEXT_WAVE_POLICY::ALL_DEAD;
			else if ("TIMER" == fields[6])
				wave.eNextWavePolicy = SPAWN_NEXT_WAVE_POLICY::TIMER;
			else
			{
				m_strStatus = "Spawn group next wave policy is unknown";
				return false;
			}
			if ((SPAWN_NEXT_WAVE_POLICY::TIMER == wave.eNextWavePolicy) != (0u != wave.iNextWaveDelayMs))
			{
				m_strStatus = "Spawn group wave delay contradicts its policy";
				return false;
			}
			wave.strWaveId = fields[2];
			wave.Entries.reserve(entryCount);
			// 뒤 ENTRY가 group+wave ID로 찾을 위치다. emplace 결과는 확인하지 않으므로 같은 wave ID가 중복되지 않는 저작 계약은 publisher 검증에 의존한다.
			waveIndices.emplace(std::string(fields[1]) + "\n" + wave.strWaveId,
			                    std::make_pair(group->second, waveIndex));
			stagedGroups[group->second].Waves.push_back(std::move(wave));
		}
		// 먼저 등록된 wave에 0부터 연속하는 entry index로 붙인다. profile/anchor도 이 시점에 이미 읽혀 있어야 한다.
		else if (!fields.empty() && "ENTRY" == fields[0])
		{
			std::uint32_t entryIndex = 0;
			SPAWN_GROUP_ENTRY entry;
			const auto wave = 3u <= fields.size()
			                      ? waveIndices.find(std::string(fields[1]) + "\n" + std::string(fields[2]))
			                      : waveIndices.end();
			if (9u != fields.size() || waveIndices.end() == wave || !ParseNumber(fields[3], entryIndex) ||
			    entryIndex != stagedGroups[wave->second.first].Waves[wave->second.second].Entries.size() ||
			    !ParseNumber(fields[5], entry.iCount) || 0u == entry.iCount ||
			    !ParseNumber(fields[7], entry.iInitialDelayMs) || !ParseNumber(fields[8], entry.iSpawnIntervalMs))
			{
				m_strStatus = "Spawn group entry row is invalid";
				return false;
			}
			entry.strArchetypeId = fields[4];
			entry.strAnchorId = fields[6];
			if (!stagedProfiles.contains(entry.strArchetypeId) || !stagedAnchors.contains(entry.strAnchorId))
			{
				m_strStatus = "Spawn group entry references unknown runtime data";
				return false;
			}
			stagedGroups[wave->second.first].Waves[wave->second.second].Entries.push_back(std::move(entry));
		}
		else
		{
			m_strStatus = "Unknown spawn group bootstrap row";
			return false;
		}
	}
	// 헤더의 상위 profile/anchor/group 수와 실제 수를 대조하고 비어 있는 wave/entry 목록을 거부한다.
	// GROUP/WAVE에 적힌 하위 개수와의 일치나 getline 실패 원인의 EOF/IO 오류 구분까지 검사하지는 않는다.
	if (stagedProfiles.size() != profileCount || stagedAnchors.size() != anchorCount ||
	    stagedGroups.size() != groupCount ||
	    std::any_of(stagedGroups.begin(), stagedGroups.end(),
	                [](const SPAWN_GROUP_DEFINITION& group)
	                {
		                return group.Waves.empty() ||
		                       std::any_of(group.Waves.begin(), group.Waves.end(),
		                                   [](const SPAWN_GROUP_WAVE& wave) { return wave.Entries.empty(); });
	                }))
	{
		m_strStatus = "Spawn group bootstrap counts do not match its rows";
		return false;
	}
	// 선행 group은 뒤에 선언해도 되므로 모든 행을 읽은 뒤 존재를 확인한다. 자기 참조/순환 검사는 publisher에 있으며 이 루프는 존재만 검사한다.
	for (const auto& group : stagedGroups)
		if (!group.strRequiredCompletedGroupId.empty() && !groupIndices.contains(group.strRequiredCompletedGroupId))
		{
			m_strStatus = "Spawn group prerequisite is unknown";
			return false;
		}
	// 정의 저장소를 교체하면 기존 runtime의 정의 포인터는 더 이상 사용하지 않는다. 룸은 Load 성공 뒤 SpawnGroupRuntime을 다시 초기화한다.
	m_Groups = std::move(stagedGroups);
	m_Anchors = std::move(stagedAnchors);
	m_Profiles = std::move(stagedProfiles);
	m_iRevision = revision;
	m_strStatus = "Loaded spawn groups: " + std::to_string(m_Groups.size());
	return true;
}

// 생성 callback과 runtime 참조 검사가 현재 anchor 값을 차용한다. 등록되지 않은 ID는 nullptr다.
const LostArk::Server::SPAWN_GROUP_ANCHOR* LostArk::Server::CSpawnGroupBootstrap::Find_Anchor(
    const std::string& anchorId) const
{
	const auto iter = m_Anchors.find(anchorId);
	return m_Anchors.end() == iter ? nullptr : &iter->second;
}

// Spawn_Monster가 자기 개체에 수치를 복사하기 위한 비소유 조회다. 현재 bootstrap 저장소가 포인터의 수명을 결정한다.
const LostArk::Server::MONSTER_RUNTIME_PROFILE* LostArk::Server::CSpawnGroupBootstrap::Find_Profile(
    const std::string& archetypeId) const
{
	const auto iter = m_Profiles.find(archetypeId);
	return m_Profiles.end() == iter ? nullptr : &iter->second;
}
