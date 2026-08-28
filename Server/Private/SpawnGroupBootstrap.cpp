#include "SpawnGroupBootstrap.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <unordered_set>

namespace
{
	using namespace LostArk::Server;
	using LostArk::Shared::WORLD_ID;
	constexpr float MAX_ABSOLUTE_ANCHOR_COMPONENT = 100000.f;

	bool IsValidAnchorComponent(const float value)
	{
		return std::isfinite(value) &&
			std::fabs(value) <= MAX_ABSOLUTE_ANCHOR_COMPONENT;
	}

	std::string_view World_ToString(const WORLD_ID worldId)
	{
		switch (worldId)
		{
		case WORLD_ID::BERN: return "BERN";
		case WORLD_ID::VALTAN_ARENA: return "VALTAN_ARENA";
		case WORLD_ID::TRAINING_GROUND: return "TRAINING_GROUND";
		case WORLD_ID::CHARACTER_SELECT_ARENA: return "CHARACTER_SELECT_ARENA";
		default: return {};
		}
	}

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

	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> result;
		std::string_view view(line);
		size_t start = 0;
		while (true)
		{
			const size_t tab = view.find('\t', start);
			result.push_back(view.substr(start,
				std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return result;
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto parsed = std::from_chars(value.data(), value.data() + value.size(), outValue);
		return std::errc{} == parsed.ec && parsed.ptr == value.data() + value.size();
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}
}

bool LostArk::Server::CSpawnGroupBootstrap::Load(
	const LostArk::Shared::WORLD_ID worldId)
{
	const std::string_view worldName = World_ToString(worldId);
	if (worldName.empty())
	{
		m_strStatus = "Unknown spawn group world ID";
		return false;
	}
	const std::filesystem::path path = Resolve_DataRoot() / L"World" /
		std::filesystem::path(std::string(worldName) + ".spawngroupsbootstrap");
	std::ifstream input(path, std::ios::binary);
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
	const auto header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t revision = 0;
	std::uint32_t anchorCount = 0;
	std::uint32_t groupCount = 0;
	std::uint32_t profileCount = 0;
	if (8u != header.size() || "LOSTARK_SPAWN_GROUP_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 4u != version || header[2] != worldName ||
		!ParseNumber(header[4], revision) || 0u == revision ||
		!ParseNumber(header[5], anchorCount) || anchorCount > 128u ||
		!ParseNumber(header[6], groupCount) || groupCount > 32u ||
		!ParseNumber(header[7], profileCount) || profileCount > 128u)
	{
		m_strStatus = "Spawn group bootstrap header is invalid";
		return false;
	}

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
		if (!fields.empty() && "PROFILE" == fields[0])
		{
			MONSTER_RUNTIME_PROFILE profile;
			std::uint32_t attackKnockdownFlag = 0u;
			if (23u != fields.size() ||
				!ParseNumber(fields[2], profile.iMaxHp) || 0u == profile.iMaxHp ||
				!ParseNumber(fields[3], profile.iAttackPower) ||
				!ParseNumber(fields[4], profile.iDefense) ||
				!ParseNumber(fields[5], profile.fCollisionRadius) ||
				!ParseNumber(fields[6], profile.fEngageRange) ||
				!ParseNumber(fields[7], profile.fMoveSpeed) ||
				!ParseNumber(fields[8], profile.fAttackRange) ||
				!ParseNumber(fields[9], profile.iAttackWindupMs) ||
				!ParseNumber(fields[10], profile.iAttackActiveMs) ||
				!ParseNumber(fields[11], profile.iAttackRecoveryMs) ||
				!ParseNumber(fields[12], profile.iDeadDespawnMs) ||
				!ParseNumber(fields[13], profile.fHitKnockbackScale) ||
				!ParseNumber(fields[14], profile.fAttackPushRangeM) ||
				!ParseNumber(fields[15], profile.iAttackPushMs) ||
				!ParseNumber(fields[16], attackKnockdownFlag) ||
				!ParseNumber(fields[17], profile.iAttackDownMs) ||
				!ParseNumber(fields[18], profile.fTargetReleaseRange) ||
				!ParseNumber(fields[19], profile.fTurnSpeedDegreesPerSecond) ||
				!ParseNumber(fields[20], profile.fAcceleration) ||
				!ParseNumber(fields[21], profile.fDeceleration) ||
				!ParseNumber(fields[22], profile.fArrivalSlowRadius) ||
				!std::isfinite(profile.fCollisionRadius) || profile.fCollisionRadius <= 0.f ||
				!std::isfinite(profile.fEngageRange) || profile.fEngageRange <= 0.f ||
				!std::isfinite(profile.fMoveSpeed) || profile.fMoveSpeed <= 0.f ||
				!std::isfinite(profile.fAttackRange) || profile.fAttackRange <= 0.f ||
				!std::isfinite(profile.fTargetReleaseRange) ||
				profile.fTargetReleaseRange < profile.fEngageRange ||
				!std::isfinite(profile.fTurnSpeedDegreesPerSecond) ||
				profile.fTurnSpeedDegreesPerSecond <= 0.f ||
				!std::isfinite(profile.fAcceleration) || profile.fAcceleration <= 0.f ||
				!std::isfinite(profile.fDeceleration) || profile.fDeceleration <= 0.f ||
				!std::isfinite(profile.fArrivalSlowRadius) ||
				profile.fArrivalSlowRadius < profile.fCollisionRadius ||
				!std::isfinite(profile.fHitKnockbackScale) ||
				profile.fHitKnockbackScale < 0.f ||
				!std::isfinite(profile.fAttackPushRangeM) ||
				std::fabs(profile.fAttackPushRangeM) > 20.f ||
				attackKnockdownFlag > 1u ||
				(0.f != profile.fAttackPushRangeM && 0u == profile.iAttackPushMs) ||
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
		else if (!fields.empty() && "ANCHOR" == fields[0])
		{
			SPAWN_GROUP_ANCHOR anchor;
			if (6u != fields.size() || !ParseNumber(fields[2], anchor.fPositionX) ||
				!ParseNumber(fields[3], anchor.fPositionY) ||
				!ParseNumber(fields[4], anchor.fPositionZ) ||
				!ParseNumber(fields[5], anchor.fYawDegrees) ||
				!IsValidAnchorComponent(anchor.fPositionX) ||
				!IsValidAnchorComponent(anchor.fPositionY) ||
				!IsValidAnchorComponent(anchor.fPositionZ) ||
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
		else if (!fields.empty() && "GROUP" == fields[0])
		{
			SPAWN_GROUP_DEFINITION group;
			std::uint32_t waveCount = 0;
			if (7u != fields.size() || !ParseNumber(fields[3], group.iMaxAlive) ||
				0u == group.iMaxAlive || group.iMaxAlive > 64u ||
				!ParseNumber(fields[4], waveCount) || 0u == waveCount || waveCount > 16u ||
				!ParseNumber(fields[6], group.iRepeatDelayMs) ||
				group.iRepeatDelayMs > 600000u)
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
			/* The delay only means something to REPEAT, so a ONCE group that
			publishes one is a publisher bug rather than a value to ignore. */
			if ((SPAWN_GROUP_REPEAT_POLICY::REPEAT == group.eRepeatPolicy) !=
				(0u != group.iRepeatDelayMs))
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
		else if (!fields.empty() && "WAVE" == fields[0])
		{
			std::uint32_t waveIndex = 0;
			std::uint32_t entryCount = 0;
			SPAWN_GROUP_WAVE wave;
			const auto group = 3u <= fields.size() ? groupIndices.find(std::string(fields[1])) : groupIndices.end();
			if (8u != fields.size() || groupIndices.end() == group ||
				!ParseNumber(fields[3], waveIndex) ||
				waveIndex != stagedGroups[group->second].Waves.size() ||
				!ParseNumber(fields[4], wave.iStartDelayMs) ||
				!ParseNumber(fields[5], entryCount) || 0u == entryCount || entryCount > 16u ||
				!ParseNumber(fields[7], wave.iNextWaveDelayMs) ||
				wave.iNextWaveDelayMs > 600000u)
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
			if ((SPAWN_NEXT_WAVE_POLICY::TIMER == wave.eNextWavePolicy) !=
				(0u != wave.iNextWaveDelayMs))
			{
				m_strStatus = "Spawn group wave delay contradicts its policy";
				return false;
			}
			wave.strWaveId = fields[2];
			wave.Entries.reserve(entryCount);
			waveIndices.emplace(std::string(fields[1]) + "\n" + wave.strWaveId,
				std::make_pair(group->second, waveIndex));
			stagedGroups[group->second].Waves.push_back(std::move(wave));
		}
		else if (!fields.empty() && "ENTRY" == fields[0])
		{
			std::uint32_t entryIndex = 0;
			SPAWN_GROUP_ENTRY entry;
			const auto wave = 3u <= fields.size() ?
				waveIndices.find(std::string(fields[1]) + "\n" + std::string(fields[2])) : waveIndices.end();
			if (9u != fields.size() || waveIndices.end() == wave ||
				!ParseNumber(fields[3], entryIndex) ||
				entryIndex != stagedGroups[wave->second.first].Waves[wave->second.second].Entries.size() ||
				!ParseNumber(fields[5], entry.iCount) || 0u == entry.iCount ||
				!ParseNumber(fields[7], entry.iInitialDelayMs) ||
				!ParseNumber(fields[8], entry.iSpawnIntervalMs))
			{
				m_strStatus = "Spawn group entry row is invalid";
				return false;
			}
			entry.strArchetypeId = fields[4];
			entry.strAnchorId = fields[6];
			if (!stagedProfiles.contains(entry.strArchetypeId) ||
				!stagedAnchors.contains(entry.strAnchorId))
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
	if (stagedProfiles.size() != profileCount || stagedAnchors.size() != anchorCount ||
		stagedGroups.size() != groupCount ||
		std::any_of(stagedGroups.begin(), stagedGroups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.Waves.empty() || std::any_of(group.Waves.begin(), group.Waves.end(),
					[](const SPAWN_GROUP_WAVE& wave) { return wave.Entries.empty(); });
			}))
	{
		m_strStatus = "Spawn group bootstrap counts do not match its rows";
		return false;
	}
	for (const auto& group : stagedGroups)
		if (!group.strRequiredCompletedGroupId.empty() &&
			!groupIndices.contains(group.strRequiredCompletedGroupId))
		{
			m_strStatus = "Spawn group prerequisite is unknown";
			return false;
		}
	m_Groups = std::move(stagedGroups);
	m_Anchors = std::move(stagedAnchors);
	m_Profiles = std::move(stagedProfiles);
	m_iRevision = revision;
	m_strStatus = "Loaded spawn groups: " + std::to_string(m_Groups.size());
	return true;
}

const LostArk::Server::SPAWN_GROUP_ANCHOR*
LostArk::Server::CSpawnGroupBootstrap::Find_Anchor(const std::string& anchorId) const
{
	const auto iter = m_Anchors.find(anchorId);
	return m_Anchors.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::MONSTER_RUNTIME_PROFILE*
LostArk::Server::CSpawnGroupBootstrap::Find_Profile(const std::string& archetypeId) const
{
	const auto iter = m_Profiles.find(archetypeId);
	return m_Profiles.end() == iter ? nullptr : &iter->second;
}
