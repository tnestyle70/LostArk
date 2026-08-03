#include "GameplayCatalog.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace
{
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

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}

	bool ParseCharacterClass(
		const std::string_view value,
		LostArk::Shared::CHARACTER_CLASS_ID& output)
	{
		if ("LANCE_MASTER" != value)
			return false;
		output = LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;
		return true;
	}
}

bool LostArk::Server::CGameplayCatalog::Load()
{
	m_Skills.clear();
	m_Bosses.clear();
	m_Players.clear();
	m_DamageByProfileId.clear();

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Gameplay" / L"Gameplay.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing gameplay bootstrap: " + path.string();
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Gameplay bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() || "LOSTARK_GAMEPLAY_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 1u != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Gameplay bootstrap header is invalid";
		return false;
	}

	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Gameplay bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		if (!fields.empty() && "DAMAGE" == fields[0])
		{
			std::uint32_t amount = 0;
			if (3u != fields.size() || !IsStableId(fields[1]) ||
				!ParseNumber(fields[2], amount) || 0u == amount ||
				!m_DamageByProfileId.emplace(std::string(fields[1]), amount).second)
			{
				m_strStatus = "Damage profile row is invalid";
				return false;
			}
		}
		else if (!fields.empty() && "SKILL" == fields[0])
		{
			PLAYER_SKILL_DEFINITION skill{};
			if (12u != fields.size() ||
				!ParseNumber(fields[1], skill.iSkillId) ||
				LostArk::Shared::INVALID_SKILL_ID == skill.iSkillId ||
				!ParseCharacterClass(fields[2], skill.eCharacterClass) ||
				!IsStableId(fields[3]) || !IsStableId(fields[4]) ||
				!ParseNumber(fields[5], skill.iCooldownMs) ||
				!ParseNumber(fields[6], skill.iActionDurationMs) ||
				!ParseNumber(fields[7], skill.iHitTimeMs) ||
				!ParseNumber(fields[8], skill.iResourceCost) ||
				!ParseNumber(fields[9], skill.fMovementDistance) ||
				!ParseNumber(fields[10], skill.fMaximumRange) ||
				!IsStableId(fields[11]) ||
				0u == skill.iCooldownMs || 0u == skill.iActionDurationMs ||
				0u == skill.iHitTimeMs ||
				skill.iHitTimeMs > skill.iActionDurationMs ||
				skill.iResourceCost > 100u ||
				!std::isfinite(skill.fMovementDistance) ||
				!std::isfinite(skill.fMaximumRange) ||
				skill.fMovementDistance < 0.f || skill.fMaximumRange <= 0.f)
			{
				m_strStatus = "Player skill row is invalid";
				return false;
			}
			skill.strInputSlot = fields[3];
			skill.strActionId = fields[4];
			skill.strDamageProfileId = fields[11];
			if (!m_Skills.emplace(skill.iSkillId, std::move(skill)).second)
			{
				m_strStatus = "Duplicate player skill ID";
				return false;
			}
		}
		else if (!fields.empty() && "BOSS" == fields[0])
		{
			BOSS_RUNTIME_PROFILE boss{};
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], boss.iMaximumHp) ||
				!ParseNumber(fields[4], boss.fEngageDistance) ||
				!ParseNumber(fields[5], boss.fMoveSpeed) ||
				!ParseNumber(fields[6], boss.iPhaseTwoHpPercent) ||
				0u == boss.iMaximumHp || !std::isfinite(boss.fEngageDistance) ||
				!std::isfinite(boss.fMoveSpeed) || boss.fEngageDistance <= 0.f ||
				boss.fMoveSpeed <= 0.f || 0u == boss.iPhaseTwoHpPercent ||
				boss.iPhaseTwoHpPercent >= 100u)
			{
				m_strStatus = "Boss profile row is invalid";
				return false;
			}
			boss.strArchetypeId = fields[1];
			boss.strEncounterId = fields[2];
			if (!m_Bosses.emplace(boss.strArchetypeId, std::move(boss)).second)
			{
				m_strStatus = "Duplicate boss archetype ID";
				return false;
			}
		}
		else if (!fields.empty() && "PLAYER" == fields[0])
		{
			PLAYER_RUNTIME_PROFILE player{};
			if (5u != fields.size() ||
				!ParseCharacterClass(fields[1], player.eCharacterClass) ||
				!ParseNumber(fields[2], player.iMaximumHp) ||
				!ParseNumber(fields[3], player.iMaximumResource) ||
				!ParseNumber(fields[4], player.fMoveSpeed) ||
				0u == player.iMaximumHp || 0u == player.iMaximumResource ||
				!std::isfinite(player.fMoveSpeed) || player.fMoveSpeed <= 0.f ||
				!m_Players.emplace(player.eCharacterClass, player).second)
			{
				m_strStatus = "Player profile row is invalid";
				return false;
			}
		}
		else
		{
			m_strStatus = "Unknown gameplay bootstrap row kind";
			return false;
		}
	}

	if (std::getline(input, line) || m_Skills.empty() || m_Players.empty() ||
		m_Bosses.empty() || m_DamageByProfileId.empty())
	{
		m_strStatus = "Gameplay bootstrap has trailing rows or missing definitions";
		return false;
	}
	for (const auto& [skillId, skill] : m_Skills)
	{
		(void)skillId;
		if (0u == Find_Damage(skill.strDamageProfileId))
		{
			m_strStatus = "Player skill references missing damage profile";
			return false;
		}
	}

	m_strStatus = "Loaded gameplay bootstrap";
	return true;
}

const LostArk::Server::PLAYER_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalog::Find_Player(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	const auto iter = m_Players.find(characterClass);
	return m_Players.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::PLAYER_SKILL_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_Skill(
	const LostArk::Shared::SKILL_ID skillId) const
{
	const auto iter = m_Skills.find(skillId);
	return m_Skills.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::BOSS_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalog::Find_Boss(
	const std::string& archetypeId) const
{
	const auto iter = m_Bosses.find(archetypeId);
	return m_Bosses.end() == iter ? nullptr : &iter->second;
}

std::uint32_t LostArk::Server::CGameplayCatalog::Find_Damage(
	const std::string& damageProfileId) const
{
	const auto iter = m_DamageByProfileId.find(damageProfileId);
	return m_DamageByProfileId.end() == iter ? 0u : iter->second;
}
