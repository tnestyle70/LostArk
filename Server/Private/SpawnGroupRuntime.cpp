#include "SpawnGroupRuntime.h"

#include <algorithm>
#include <cmath>
#include <limits>

bool LostArk::Server::CSpawnGroupRuntime::Initialize(
	const CSpawnGroupBootstrap& bootstrap, std::string& outStatus)
{
	std::vector<RUNTIME_GROUP> staged;
	staged.reserve(bootstrap.Get_Groups().size());
	for (const SPAWN_GROUP_DEFINITION& definition : bootstrap.Get_Groups())
	{
		if (definition.strSpawnGroupId.empty() || 0u == definition.iMaxAlive ||
			definition.Waves.empty())
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

bool LostArk::Server::CSpawnGroupRuntime::Activate(const std::string& spawnGroupId)
{
	RUNTIME_GROUP* group = Find(spawnGroupId);
	if (nullptr == group || GROUP_STATE::DORMANT != group->eState ||
		nullptr == group->pDefinition)
		return false;
	if (!group->pDefinition->strRequiredCompletedGroupId.empty() &&
		!Is_Completed(group->pDefinition->strRequiredCompletedGroupId))
		return false;
	group->eState = GROUP_STATE::RUNNING;
	group->iWaveIndex = 0;
	Begin_Wave(*group);
	return true;
}

bool LostArk::Server::CSpawnGroupRuntime::Activate_Immediate(
	const std::string& spawnGroupId,
	const CSpawnGroupBootstrap& bootstrap,
	const SPAWN_CALLBACK& spawn)
{
	RUNTIME_GROUP* group = Find(spawnGroupId);
	if (nullptr == group || GROUP_STATE::DORMANT != group->eState ||
		nullptr == group->pDefinition || !spawn)
	{
		return false;
	}

	const SPAWN_GROUP_DEFINITION& definition = *group->pDefinition;
	if (!definition.strRequiredCompletedGroupId.empty() ||
		1u != definition.iMaxAlive || 1u != definition.Waves.size())
	{
		return false;
	}
	const SPAWN_GROUP_WAVE& wave = definition.Waves.front();
	if (0u != wave.iStartDelayMs || 1u != wave.Entries.size())
		return false;
	const SPAWN_GROUP_ENTRY& entry = wave.Entries.front();
	if (1u != entry.iCount || 0u != entry.iInitialDelayMs ||
		0u != entry.iSpawnIntervalMs)
	{
		return false;
	}
	const SPAWN_GROUP_ANCHOR* anchor =
		bootstrap.Find_Anchor(entry.strAnchorId);
	const MONSTER_RUNTIME_PROFILE* profile =
		bootstrap.Find_Profile(entry.strArchetypeId);
	if (nullptr == anchor || nullptr == profile ||
		!spawn(spawnGroupId, entry, *anchor, *profile, 0u))
	{
		return false;
	}

	group->eState = GROUP_STATE::RUNNING;
	group->iWaveIndex = 0u;
	Begin_Wave(*group);
	group->SpawnedByEntry[0] = 1u;
	return true;
}

void LostArk::Server::CSpawnGroupRuntime::Update(
	const float fixedDeltaSeconds,
	const CSpawnGroupBootstrap& bootstrap,
	const ACTIVE_COUNT_QUERY& activeCount,
	const SPAWN_CALLBACK& spawn)
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		!activeCount || !spawn)
		return;
	const std::uint64_t deltaMs = static_cast<std::uint64_t>(
		(std::max)(1.0, std::round(static_cast<double>(fixedDeltaSeconds) * 1000.0)));
	for (RUNTIME_GROUP& group : m_Groups)
	{
		if (nullptr == group.pDefinition)
			continue;
		if (GROUP_STATE::COMPLETED == group.eState)
		{
			if (SPAWN_GROUP_REPEAT_POLICY::REPEAT !=
				group.pDefinition->eRepeatPolicy)
			{
				continue;
			}
			/* A repeat waits for the field to clear before it starts again, so a
			second run never stacks on top of the one still being fought. The
			delay is then measured from that quiet moment. */
			if (0u != activeCount(group.pDefinition->strSpawnGroupId))
			{
				group.iElapsedMs = 0;
				continue;
			}
			group.iElapsedMs = (std::min)(
				(std::numeric_limits<std::uint64_t>::max)() - deltaMs,
				group.iElapsedMs) + deltaMs;
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
		const bool allScheduled = group.SpawnedByEntry.size() == wave.Entries.size() &&
			std::equal(group.SpawnedByEntry.begin(), group.SpawnedByEntry.end(),
				wave.Entries.begin(), [](const std::uint32_t spawned,
					const SPAWN_GROUP_ENTRY& entry) { return spawned >= entry.iCount; });
		std::uint32_t aliveCount = activeCount(group.pDefinition->strSpawnGroupId);
		/* ALL_DEAD waits the group out; TIMER runs on its own clock so the next
		wave can open while this one is still standing. maxAlive still caps the
		group either way, so TIMER cannot outrun the spawn budget. */
		const bool waveFinished =
			SPAWN_NEXT_WAVE_POLICY::TIMER == wave.eNextWavePolicy ?
				group.iElapsedMs >= static_cast<std::uint64_t>(wave.iStartDelayMs) +
					wave.iNextWaveDelayMs :
				allScheduled && 0u == aliveCount;
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

		group.iElapsedMs = (std::min)(
			(std::numeric_limits<std::uint64_t>::max)() - deltaMs,
			group.iElapsedMs) + deltaMs;
		if (group.iElapsedMs < wave.iStartDelayMs)
			continue;
		for (size_t entryIndex = 0; entryIndex < wave.Entries.size() &&
			aliveCount < group.pDefinition->iMaxAlive; ++entryIndex)
		{
			const SPAWN_GROUP_ENTRY& entry = wave.Entries[entryIndex];
			std::uint32_t& spawnedCount = group.SpawnedByEntry[entryIndex];
			while (spawnedCount < entry.iCount &&
				aliveCount < group.pDefinition->iMaxAlive)
			{
				const std::uint64_t dueMs =
					static_cast<std::uint64_t>(wave.iStartDelayMs) +
					entry.iInitialDelayMs +
					static_cast<std::uint64_t>(spawnedCount) * entry.iSpawnIntervalMs;
				if (group.iElapsedMs < dueMs)
					break;
				const SPAWN_GROUP_ANCHOR* anchor = bootstrap.Find_Anchor(entry.strAnchorId);
				const MONSTER_RUNTIME_PROFILE* profile =
					bootstrap.Find_Profile(entry.strArchetypeId);
				if (nullptr == anchor || nullptr == profile ||
					!spawn(group.pDefinition->strSpawnGroupId, entry,
						*anchor, *profile, spawnedCount))
					break;
				++spawnedCount;
				++aliveCount;
			}
		}
	}
}

bool LostArk::Server::CSpawnGroupRuntime::Is_Completed(
	const std::string& spawnGroupId) const
{
	const RUNTIME_GROUP* group = Find(spawnGroupId);
	return nullptr != group && GROUP_STATE::COMPLETED == group->eState;
}

bool LostArk::Server::CSpawnGroupRuntime::Is_ActiveOrCompleted(
	const std::string& spawnGroupId) const
{
	const RUNTIME_GROUP* group = Find(spawnGroupId);
	return nullptr != group && GROUP_STATE::DORMANT != group->eState;
}

LostArk::Server::CSpawnGroupRuntime::RUNTIME_GROUP*
LostArk::Server::CSpawnGroupRuntime::Find(const std::string& spawnGroupId)
{
	const auto iter = std::find_if(m_Groups.begin(), m_Groups.end(),
		[&](const RUNTIME_GROUP& value)
		{
			return nullptr != value.pDefinition &&
				value.pDefinition->strSpawnGroupId == spawnGroupId;
		});
	return m_Groups.end() == iter ? nullptr : &*iter;
}

const LostArk::Server::CSpawnGroupRuntime::RUNTIME_GROUP*
LostArk::Server::CSpawnGroupRuntime::Find(const std::string& spawnGroupId) const
{
	const auto iter = std::find_if(m_Groups.begin(), m_Groups.end(),
		[&](const RUNTIME_GROUP& value)
		{
			return nullptr != value.pDefinition &&
				value.pDefinition->strSpawnGroupId == spawnGroupId;
		});
	return m_Groups.end() == iter ? nullptr : &*iter;
}

void LostArk::Server::CSpawnGroupRuntime::Begin_Wave(RUNTIME_GROUP& group)
{
	group.iElapsedMs = 0;
	group.SpawnedByEntry.clear();
	if (nullptr != group.pDefinition &&
		group.iWaveIndex < group.pDefinition->Waves.size())
	{
		group.SpawnedByEntry.resize(
			group.pDefinition->Waves[group.iWaveIndex].Entries.size(), 0u);
	}
}
