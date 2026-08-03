#include "WorldBootstrap.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <unordered_set>

namespace
{
	using namespace LostArk::Server;
	using LostArk::Shared::WORLD_ID;

	std::string_view World_ToString(const WORLD_ID worldId)
	{
		switch (worldId)
		{
		case WORLD_ID::BERN: return "BERN";
		case WORLD_ID::VALTAN_ARENA: return "VALTAN_ARENA";
		case WORLD_ID::TRAINING_GROUND: return "TRAINING_GROUND";
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
		std::vector<std::string_view> values;
		std::string_view view(line);
		size_t start = 0u;
		while (true)
		{
			const size_t tab = view.find('\t', start);
			values.push_back(view.substr(start,
				std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return values;
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), outValue);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	bool ParseKind(
		const std::string_view value,
		WORLD_BOOTSTRAP_KIND& outKind)
	{
		if ("playerSpawn" == value)
			outKind = WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN;
		else if ("npc" == value)
			outKind = WORLD_BOOTSTRAP_KIND::NPC;
		else if ("boss" == value)
			outKind = WORLD_BOOTSTRAP_KIND::BOSS;
		else
			return false;
		return true;
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
}

bool LostArk::Server::CWorldBootstrap::Load(
	const LostArk::Shared::WORLD_ID worldId)
{
	m_Placements.clear();
	m_strAreaId.clear();
	m_iRevision = 0;

	const std::string_view worldName = World_ToString(worldId);
	if (worldName.empty())
	{
		m_strStatus = "Unknown world ID";
		return false;
	}
	const std::filesystem::path dataRoot = Resolve_DataRoot();
	if (dataRoot.empty())
	{
		m_strStatus = "Could not resolve server data root";
		return false;
	}
	const std::filesystem::path path = dataRoot / L"World" /
		std::filesystem::path(std::string(worldName) + ".worldbootstrap");
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		m_strStatus = "Missing world bootstrap: " + path.string();
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Empty world bootstrap";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t revision = 0;
	std::uint32_t count = 0;
	if (6u != header.size() ||
		"LOSTARK_WORLD_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 2u != version ||
		header[2] != worldName || !IsStableId(header[3]) ||
		!ParseNumber(header[4], revision) || 0u == revision ||
		!ParseNumber(header[5], count) || count > 4096u)
	{
		m_strStatus = "World bootstrap header is invalid";
		return false;
	}
	// Header fields are string_views into `line`. Preserve the area ID before
	// subsequent getline calls reuse that string buffer for placement rows.
	const std::string stagedAreaId{ header[3] };

	std::vector<WORLD_BOOTSTRAP_PLACEMENT> staged;
	std::unordered_set<std::string> placementIds;
	staged.reserve(count);
	for (std::uint32_t index = 0; index < count; ++index)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "World bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		WORLD_BOOTSTRAP_PLACEMENT placement;
		int enabled = 0;
		if (17u != fields.size() ||
			!IsStableId(fields[0]) || !ParseKind(fields[1], placement.eKind) ||
			("-" != fields[2] && !IsStableId(fields[2])) ||
			("-" != fields[3] && !IsStableId(fields[3])) ||
			!ParseNumber(fields[4], placement.fPositionX) ||
			!ParseNumber(fields[5], placement.fPositionY) ||
			!ParseNumber(fields[6], placement.fPositionZ) ||
			!ParseNumber(fields[7], placement.fYawDegrees) ||
			!ParseNumber(fields[8], enabled) || (0 != enabled && 1 != enabled) ||
			("-" != fields[9] && !IsStableId(fields[9])) ||
			("-" != fields[10] && !IsStableId(fields[10])) ||
			!ParseNumber(fields[11], placement.fPatternMinimumRange) ||
			!ParseNumber(fields[12], placement.fPatternMaximumRange) ||
			!ParseNumber(fields[13], placement.iPatternTelegraphMs) ||
			!ParseNumber(fields[14], placement.iPatternActiveMs) ||
			!ParseNumber(fields[15], placement.iPatternRecoveryMs) ||
			("-" != fields[16] && !IsStableId(fields[16])) ||
			!std::isfinite(placement.fPositionX) ||
			!std::isfinite(placement.fPositionY) ||
			!std::isfinite(placement.fPositionZ) ||
			!std::isfinite(placement.fYawDegrees) ||
			!std::isfinite(placement.fPatternMinimumRange) ||
			!std::isfinite(placement.fPatternMaximumRange))
		{
			m_strStatus = "World bootstrap placement is invalid at row " +
				std::to_string(index);
			return false;
		}
		placement.strPlacementId = fields[0];
		placement.strArchetypeId =
			"-" == fields[2] ? "" : std::string(fields[2]);
		placement.strEncounterId = "-" == fields[3] ? "" : std::string(fields[3]);
		placement.strPatternId = "-" == fields[9] ? "" : std::string(fields[9]);
		placement.strActionId = "-" == fields[10] ? "" : std::string(fields[10]);
		placement.strDamageProfileId = "-" == fields[16] ? "" : std::string(fields[16]);
		placement.isEnabled = 1 == enabled;
		const bool isBoss = WORLD_BOOTSTRAP_KIND::BOSS == placement.eKind;
		const bool isPlayerSpawn =
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
		if ((isPlayerSpawn && !placement.strArchetypeId.empty()) ||
			(!isPlayerSpawn && placement.strArchetypeId.empty()))
		{
			m_strStatus = "World bootstrap archetype contract is invalid at row " +
				std::to_string(index);
			return false;
		}
		const bool hasValidBossPattern =
			!placement.strEncounterId.empty() &&
			!placement.strPatternId.empty() &&
			!placement.strActionId.empty() &&
			!placement.strDamageProfileId.empty() &&
			placement.fPatternMinimumRange >= 0.f &&
			placement.fPatternMaximumRange > placement.fPatternMinimumRange &&
			placement.fPatternMaximumRange <= 100000.f &&
			placement.iPatternTelegraphMs > 0u &&
			placement.iPatternTelegraphMs <= 600000u &&
			placement.iPatternActiveMs > 0u &&
			placement.iPatternActiveMs <= 600000u &&
			placement.iPatternRecoveryMs > 0u &&
			placement.iPatternRecoveryMs <= 600000u;
		const bool hasNoPattern =
			placement.strPatternId.empty() &&
			placement.strActionId.empty() &&
			placement.strDamageProfileId.empty() &&
			0.f == placement.fPatternMinimumRange &&
			0.f == placement.fPatternMaximumRange &&
			0u == placement.iPatternTelegraphMs &&
			0u == placement.iPatternActiveMs &&
			0u == placement.iPatternRecoveryMs;
		if ((isBoss && !hasValidBossPattern) || (!isBoss && !hasNoPattern))
		{
			m_strStatus = "World bootstrap pattern contract is invalid at row " +
				std::to_string(index);
			return false;
		}
		if (!placementIds.insert(placement.strPlacementId).second)
		{
			m_strStatus = "Duplicate world bootstrap placement ID";
			return false;
		}
		staged.push_back(std::move(placement));
	}
	if (std::getline(input, line))
	{
		m_strStatus = "World bootstrap has trailing rows";
		return false;
	}

	m_Placements = std::move(staged);
	m_strAreaId = stagedAreaId;
	m_iRevision = revision;
	m_strStatus = "Loaded world bootstrap: " +
		std::to_string(m_Placements.size()) + " placements";
	return true;
}
