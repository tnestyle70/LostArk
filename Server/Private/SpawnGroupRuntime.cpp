// 룸 callback과 게시 그룹 정의로 스케줄 상태 전이를 구현한다.
#include "SpawnGroupRuntime.h"

// 완료 횟수 비교·ID 조회·경과 시간의 상한 제한을 사용한다.
#include <algorithm>
// dt의 유한성 검사와 초→ms 반올림을 사용한다.
#include <cmath>
// uint64 경과 ms 누적이 넘치지 않도록 최댓값을 사용한다.
#include <limits>

// 룸 초기화/리셋에서 게시 정의의 최소 필드와 anchor/profile 참조를 확인한다.
// 지역 실행 목록에 정의 주소를 기록하고 성공할 때만 m_Groups를 교체하므로 명시적 false는 기존 스케줄을 유지한다.
bool LostArk::Server::CSpawnGroupRuntime::Initialize(const CSpawnGroupBootstrap& bootstrap, std::string& outStatus)
{
	// 몬스터를 복사하는 것이 아니라 bootstrap의 정의를 차용하는 DORMANT 상태를 만든다. 이 검사는 bootstrap 자체의 전체 schema 검증을 대신하지 않는다.
	std::vector<RUNTIME_GROUP> staged;
	staged.reserve(bootstrap.Get_Groups().size());
	for (const SPAWN_GROUP_DEFINITION& definition : bootstrap.Get_Groups())
	{
		if (definition.strSpawnGroupId.empty() || 0u == definition.iMaxAlive || definition.Waves.empty())
		{
			outStatus = "Spawn group runtime definition is invalid";
			return false;
		}
		for (const SPAWN_GROUP_WAVE& wave : definition.Waves)
			for (const SPAWN_GROUP_ENTRY& entry : wave.Entries)
				if (nullptr == bootstrap.Find_Anchor(entry.strAnchorId) ||
				    nullptr == bootstrap.Find_Profile(entry.strArchetypeId))
				{
					outStatus = "Spawn group runtime reference is missing";
					return false;
				}
		RUNTIME_GROUP runtime;
		runtime.pDefinition = &definition;
		staged.push_back(std::move(runtime));
	}
	m_Groups = std::move(staged);
	outStatus = "Initialized spawn group runtime: " + std::to_string(m_Groups.size());
	return true;
}

// 룸의 개체 정리가 끝난 뒤 실행 상태를 초기값으로 되돌린다. 정의 주소만 보존하며 실제 몬스터는 여기서 제거하지 않는다.
bool LostArk::Server::CSpawnGroupRuntime::Reset_Group(const std::string& spawnGroupId)
{
	RUNTIME_GROUP* group = Find(spawnGroupId);
	if (!group || !group->pDefinition)
		return false;
	const auto* definition = group->pDefinition;
	*group = {};
	group->pDefinition = definition;
	return true;
}

// 휴면 상태와 선행 그룹 완료를 검사한 뒤 첫 웨이브를 예약한다. 상태/커서 변경 전의 거절은 기존 스케줄을 보존한다.
bool LostArk::Server::CSpawnGroupRuntime::Activate(const std::string& spawnGroupId)
{
	RUNTIME_GROUP* group = Find(spawnGroupId);
	if (nullptr == group || GROUP_STATE::DORMANT != group->eState || nullptr == group->pDefinition)
		return false;
	if (!group->pDefinition->strRequiredCompletedGroupId.empty() &&
	    !Is_Completed(group->pDefinition->strRequiredCompletedGroupId))
		return false;
	group->eState = GROUP_STATE::RUNNING;
	group->iWaveIndex = 0;
	Begin_Wave(*group);
	return true;
}

// trigger가 같은 그룹을 다시 요청할 때 실행 중 웨이브 위에 또 생성하지 않도록 상태를 나눈다.
// 완료 그룹의 명시적 재시작은 자동 repeat 지연을 기다리지 않지만 잔류 개체와 선행 그룹 조건은 확인한다.
bool LostArk::Server::CSpawnGroupRuntime::Activate_Repeat(const std::string& spawnGroupId,
                                                          const ACTIVE_COUNT_QUERY& activeCount)
{
	RUNTIME_GROUP* group = Find(spawnGroupId);
	if (nullptr == group || nullptr == group->pDefinition)
		return false;
	if (GROUP_STATE::RUNNING == group->eState)
		return false;
	if (GROUP_STATE::DORMANT == group->eState)
		return Activate(spawnGroupId);
	// COMPLETED 재시작은 개체가 모두 제거되고 선행 그룹도 완료된 뒤에만 상태를 바꾼다. 거절되면 COMPLETED를 유지한다.
	if (!activeCount || 0u != activeCount(spawnGroupId))
		return false;
	if (!group->pDefinition->strRequiredCompletedGroupId.empty() &&
	    !Is_Completed(group->pDefinition->strRequiredCompletedGroupId))
		return false;
	group->eState = GROUP_STATE::RUNNING;
	group->iWaveIndex = 0;
	Begin_Wave(*group);
	return true;
}

// 룸 저작 spawn 요청의 즉시 결과를 위해 제한된 단일 개체 정의만 허용한다.
// 실제 spawn callback 성공 뒤에만 그룹을 진행 상태로 바꾼다. callback 이후 할당 예외까지 되돌리는 guard는 없다.
bool LostArk::Server::CSpawnGroupRuntime::Activate_Immediate(const std::string& spawnGroupId,
                                                             const CSpawnGroupBootstrap& bootstrap,
                                                             const SPAWN_CALLBACK& spawn)
{
	RUNTIME_GROUP* group = Find(spawnGroupId);
	if (nullptr == group || GROUP_STATE::DORMANT != group->eState || nullptr == group->pDefinition || !spawn)
	{
		return false;
	}

	// 선행 그룹 없이 1wave/1entry/1개체, 시작·entry·생성 간격 지연 모두 0인 계약만 처리한다.
	const SPAWN_GROUP_DEFINITION& definition = *group->pDefinition;
	if (!definition.strRequiredCompletedGroupId.empty() || 1u != definition.iMaxAlive || 1u != definition.Waves.size())
	{
		return false;
	}
	const SPAWN_GROUP_WAVE& wave = definition.Waves.front();
	if (0u != wave.iStartDelayMs || 1u != wave.Entries.size())
		return false;
	const SPAWN_GROUP_ENTRY& entry = wave.Entries.front();
	if (1u != entry.iCount || 0u != entry.iInitialDelayMs || 0u != entry.iSpawnIntervalMs)
	{
		return false;
	}
	// 생성에 필요한 anchor/profile을 Initialize 때와 같은 bootstrap에서 다시 차용한다. false이면 아직 스케줄은 휴면이다.
	const SPAWN_GROUP_ANCHOR* anchor = bootstrap.Find_Anchor(entry.strAnchorId);
	const MONSTER_RUNTIME_PROFILE* profile = bootstrap.Find_Profile(entry.strArchetypeId);
	if (nullptr == anchor || nullptr == profile || !spawn(spawnGroupId, entry, *anchor, *profile, 0u))
	{
		return false;
	}

	group->eState = GROUP_STATE::RUNNING;
	group->iWaveIndex = 0u;
	Begin_Wave(*group);
	group->SpawnedByEntry[0] = 1u;
	return true;
}

// GameRoom 고정 tick이 스케줄을 진행한다. 실제 Count_SpawnGroupEntities는 사망 직후 제거 대기 중인 개체도 포함한다.
// spawn true만 성공 횟수/잔류 수에 반영하며 callback 실패 시 이전에 성공한 다른 생성까지 되돌리지는 않는다.
void LostArk::Server::CSpawnGroupRuntime::Update(const float fixedDeltaSeconds, const CSpawnGroupBootstrap& bootstrap,
                                                 const ACTIVE_COUNT_QUERY& activeCount, const SPAWN_CALLBACK& spawn)
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f || !activeCount || !spawn)
		return;
	// 초를 반올림한 ms로 바꾸되 작은 양수 dt도 최소 1ms 진행한다. 정상 룸의 고정 tick 입력을 전제로 한다.
	const std::uint64_t deltaMs =
	    static_cast<std::uint64_t>((std::max)(1.0, std::round(static_cast<double>(fixedDeltaSeconds) * 1000.0)));
	for (RUNTIME_GROUP& group : m_Groups)
	{
		if (nullptr == group.pDefinition)
			continue;
		if (GROUP_STATE::COMPLETED == group.eState)
		{
			if (SPAWN_GROUP_REPEAT_POLICY::REPEAT != group.pDefinition->eRepeatPolicy)
			{
				continue;
			}
			// 자동 REPEAT는 그룹 개체가 남아 있으면 지연 시계를 매번 0으로 되돌린다.
			// 개체 수가 0인 tick부터 지연을 누적하고 첫 웨이브로 재시작한 tick에는 생성 루프를 실행하지 않는다.
			if (0u != activeCount(group.pDefinition->strSpawnGroupId))
			{
				group.iElapsedMs = 0;
				continue;
			}
			group.iElapsedMs =
			    (std::min)((std::numeric_limits<std::uint64_t>::max)() - deltaMs, group.iElapsedMs) + deltaMs;
			if (group.iElapsedMs < group.pDefinition->iRepeatDelayMs)
				continue;
			group.eState = GROUP_STATE::RUNNING;
			group.iWaveIndex = 0;
			Begin_Wave(group);
			continue;
		}
		if (GROUP_STATE::RUNNING != group.eState)
			continue;
		if (group.iWaveIndex >= group.pDefinition->Waves.size())
		{
			group.eState = GROUP_STATE::COMPLETED;
			group.iElapsedMs = 0;
			continue;
		}
		const SPAWN_GROUP_WAVE& wave = group.pDefinition->Waves[group.iWaveIndex];
		const bool allScheduled =
		    group.SpawnedByEntry.size() == wave.Entries.size() &&
		    std::equal(group.SpawnedByEntry.begin(), group.SpawnedByEntry.end(), wave.Entries.begin(),
		               [](const std::uint32_t spawned, const SPAWN_GROUP_ENTRY& entry)
		               { return spawned >= entry.iCount; });
		std::uint32_t aliveCount = activeCount(group.pDefinition->strSpawnGroupId);
		// ALL_DEAD는 현재 웨이브의 모든 생성 성공과 그룹 개체 수 0을 함께 기다린다.
		// TIMER는 생성 완료와 무관하게 웨이브 시작 지연+전환 지연으로 끝나므로 아직 생성하지 못한 entry가 남아도 다음 wave로 넘어간다.
		// 두 정책 모두 실제 생성 때 그룹 iMaxAlive를 적용하며 완료 상태 자체가 모든 개체의 제거를 뜻하지는 않는다.
		// 완료 판단은 이번 dt를 누적하기 전의 경과 값으로 한다. 다음 wave로 바꾼 tick에는 시간을 새로 시작하고 바로 생성하지 않는다.
		const bool waveFinished =
		    SPAWN_NEXT_WAVE_POLICY::TIMER == wave.eNextWavePolicy
		        ? group.iElapsedMs >= static_cast<std::uint64_t>(wave.iStartDelayMs) + wave.iNextWaveDelayMs
		        : allScheduled && 0u == aliveCount;
		if (waveFinished)
		{
			++group.iWaveIndex;
			if (group.iWaveIndex >= group.pDefinition->Waves.size())
			{
				group.eState = GROUP_STATE::COMPLETED;
				group.iElapsedMs = 0;
				continue;
			}
			Begin_Wave(group);
			continue;
		}

		group.iElapsedMs =
		    (std::min)((std::numeric_limits<std::uint64_t>::max)() - deltaMs, group.iElapsedMs) + deltaMs;
		if (group.iElapsedMs < wave.iStartDelayMs)
			continue;
		// 게시 entry 순서로 이미 기한이 지난 생성들을 처리하되 잔류 개체 한도에 도달하면 멈춘다.
		// ordinal은 해당 entry에서 지금까지 성공한 횟수이며 다음 예정 시각 계산에도 사용한다.
		for (size_t entryIndex = 0; entryIndex < wave.Entries.size() && aliveCount < group.pDefinition->iMaxAlive;
		     ++entryIndex)
		{
			const SPAWN_GROUP_ENTRY& entry = wave.Entries[entryIndex];
			std::uint32_t& spawnedCount = group.SpawnedByEntry[entryIndex];
			while (spawnedCount < entry.iCount && aliveCount < group.pDefinition->iMaxAlive)
			{
				const std::uint64_t dueMs = static_cast<std::uint64_t>(wave.iStartDelayMs) + entry.iInitialDelayMs +
				                            static_cast<std::uint64_t>(spawnedCount) * entry.iSpawnIntervalMs;
				if (group.iElapsedMs < dueMs)
					break;
				const SPAWN_GROUP_ANCHOR* anchor = bootstrap.Find_Anchor(entry.strAnchorId);
				const MONSTER_RUNTIME_PROFILE* profile = bootstrap.Find_Profile(entry.strArchetypeId);
				// 참조/생성 실패는 현재 entry의 while만 멈춘다. 성공 수는 올리지 않으며 다음 entry는 남은 한도 안에서 계속 시도할 수 있다.
				if (nullptr == anchor || nullptr == profile ||
				    !spawn(group.pDefinition->strSpawnGroupId, entry, *anchor, *profile, spawnedCount))
					break;
				++spawnedCount;
				++aliveCount;
			}
		}
	}
}

// 선행 그룹 조건과 소비자의 스케줄 상태 조회다. 남아 있는 몬스터 수는 따로 확인해야 한다.
bool LostArk::Server::CSpawnGroupRuntime::Is_Completed(const std::string& spawnGroupId) const
{
	const RUNTIME_GROUP* group = Find(spawnGroupId);
	return nullptr != group && GROUP_STATE::COMPLETED == group->eState;
}

// 활성화 이력이 있는 현재 스케줄인지 확인한다. 실제 몬스터의 생성 성공 여부를 이 값 하나로 대체하지 않는다.
bool LostArk::Server::CSpawnGroupRuntime::Is_ActiveOrCompleted(const std::string& spawnGroupId) const
{
	const RUNTIME_GROUP* group = Find(spawnGroupId);
	return nullptr != group && GROUP_STATE::DORMANT != group->eState;
}

// 변경 경로가 그룹 ID로 멤버 실행 상태를 차용한다. 없으면 nullptr이며 Initialize로 목록을 교체하면 기존 포인터를 재사용하지 않는다.
LostArk::Server::CSpawnGroupRuntime::RUNTIME_GROUP* LostArk::Server::CSpawnGroupRuntime::Find(
    const std::string& spawnGroupId)
{
	const auto iter =
	    std::find_if(m_Groups.begin(), m_Groups.end(), [&](const RUNTIME_GROUP& value)
	                 { return nullptr != value.pDefinition && value.pDefinition->strSpawnGroupId == spawnGroupId; });
	return m_Groups.end() == iter ? nullptr : &*iter;
}

// const 상태 조회가 같은 정의 ID로 실행 상태를 찾는다. bootstrap 정의 주소와 m_Groups 수명이 유지되는 동안만 반환 포인터가 유효하다.
const LostArk::Server::CSpawnGroupRuntime::RUNTIME_GROUP* LostArk::Server::CSpawnGroupRuntime::Find(
    const std::string& spawnGroupId) const
{
	const auto iter =
	    std::find_if(m_Groups.begin(), m_Groups.end(), [&](const RUNTIME_GROUP& value)
	                 { return nullptr != value.pDefinition && value.pDefinition->strSpawnGroupId == spawnGroupId; });
	return m_Groups.end() == iter ? nullptr : &*iter;
}

// 현재 wave의 entry 수에 맞춰 성공 횟수를 0으로 만들고 경과 시간을 다시 시작한다.
// wave index/상태는 호출자가 관리하며 누락 정의나 범위 밖 wave는 빈 횟수 목록만 남긴다.
void LostArk::Server::CSpawnGroupRuntime::Begin_Wave(RUNTIME_GROUP& group)
{
	group.iElapsedMs = 0;
	group.SpawnedByEntry.clear();
	if (nullptr != group.pDefinition && group.iWaveIndex < group.pDefinition->Waves.size())
	{
		group.SpawnedByEntry.resize(group.pDefinition->Waves[group.iWaveIndex].Entries.size(), 0u);
	}
}
