#include "VehicleCatalog.h"

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

	bool ParseSlot(const std::string_view value, LostArk::Server::VEHICLE_SKILL_SLOT& output)
	{
		using LostArk::Server::VEHICLE_SKILL_SLOT;
		if ("SPACE" == value) output = VEHICLE_SKILL_SLOT::SPACE;
		else if ("Q" == value) output = VEHICLE_SKILL_SLOT::Q;
		else if ("W" == value) output = VEHICLE_SKILL_SLOT::W;
		else if ("E" == value) output = VEHICLE_SKILL_SLOT::E;
		else return false;
		return true;
	}

	/* "timeMs:forward:lateral:up" tokens joined by commas, strictly increasing in
	time and inside the action length. */
	bool ParseRootMotion(
		const std::string_view packed,
		const std::uint32_t sampleCount,
		const std::uint32_t limitMs,
		std::vector<LostArk::Server::ROOT_MOTION_SAMPLE>& outSamples)
	{
		outSamples.clear();
		if (0u == sampleCount)
			return "-" == packed;
		if (sampleCount < 2u || sampleCount > 512u)
			return false;
		std::size_t cursor = 0u;
		bool reachedEnd = false;
		while (!reachedEnd && outSamples.size() < sampleCount)
		{
			const std::size_t comma = packed.find(',', cursor);
			const std::string_view token = packed.substr(
				cursor, std::string_view::npos == comma ? std::string_view::npos : comma - cursor);
			const std::size_t first = token.find(':');
			const std::size_t second = std::string_view::npos == first ? first : token.find(':', first + 1u);
			const std::size_t third = std::string_view::npos == second ? second : token.find(':', second + 1u);
			LostArk::Server::ROOT_MOTION_SAMPLE sample{};
			if (std::string_view::npos == third ||
				std::string_view::npos != token.find(':', third + 1u) ||
				!ParseNumber(token.substr(0u, first), sample.iTimeMs) ||
				!ParseNumber(token.substr(first + 1u, second - first - 1u), sample.fForward) ||
				!ParseNumber(token.substr(second + 1u, third - second - 1u), sample.fLateral) ||
				!ParseNumber(token.substr(third + 1u), sample.fUp) ||
				!std::isfinite(sample.fForward) || !std::isfinite(sample.fLateral) ||
				!std::isfinite(sample.fUp) || sample.iTimeMs > limitMs ||
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

bool LostArk::Server::CVehicleCatalog::Load()
{
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
	if (3u != header.size() || "LOSTARK_VEHICLE_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || (2u != version && 3u != version) ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Vehicle bootstrap header is invalid";
		return false;
	}

	for (std::uint32_t row = 0u; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Vehicle bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		SERVER_VEHICLE_DEFINITION vehicle{};
		std::uint32_t skillCount = 0u;
		if ((version == 3u ? 10u : 4u) != fields.size() || "VEHICLE" != fields[0] ||
			!ParseNumber(fields[1], vehicle.iVehicleId) ||
			LostArk::Shared::INVALID_VEHICLE_ID == vehicle.iVehicleId ||
			!ParseNumber(fields[2], vehicle.fMoveSpeed) ||
			!std::isfinite(vehicle.fMoveSpeed) ||
			vehicle.fMoveSpeed <= 0.f || vehicle.fMoveSpeed > 30.f ||
			!ParseNumber(fields[3], skillCount) || skillCount > 4u ||
			row + skillCount >= rowCount)
		{
			m_strStatus = "Vehicle bootstrap row is invalid";
			return false;
		}
		if (version == 3u)
		{
			float* flight[] = { &vehicle.fFlightTakeoffSeconds, &vehicle.fFlightLandingSeconds,
				&vehicle.fFlightHoverHeight, &vehicle.fFlightMaximumHeight,
				&vehicle.fFlightSpeed, &vehicle.fFlightVerticalSpeed };
			for (std::size_t index = 0u; index < std::size(flight); ++index)
				if (!ParseNumber(fields[index + 4u], *flight[index]) || !std::isfinite(*flight[index]) ||
					*flight[index] < 0.f || *flight[index] > 60.f) return false;
			if (vehicle.Has_Flight() && (vehicle.iVehicleId != LostArk::Shared::ANCIENT_SEA_VEHICLE_ID ||
				vehicle.fFlightLandingSeconds <= 0.f || vehicle.fFlightHoverHeight < .1f ||
				vehicle.fFlightMaximumHeight < vehicle.fFlightHoverHeight ||
				vehicle.fFlightSpeed <= 0.f || vehicle.fFlightVerticalSpeed <= 0.f ||
				vehicle.fFlightMaximumHeight / vehicle.fFlightVerticalSpeed > 600.f)) return false;
			if (!vehicle.Has_Flight()) for (const auto field : flight) if (*field != 0.f) return false;
		}
		std::uint8_t usedSlots = 0u;
		std::vector<LostArk::Shared::SKILL_ID> skillIds;
		for (std::uint32_t skillRow = 0u; skillRow < skillCount; ++skillRow)
		{
			if (!std::getline(input, line))
			{
				m_strStatus = "Vehicle bootstrap row is truncated";
				return false;
			}
			++row;
			StripCarriageReturn(line);
			const std::vector<std::string_view> skillFields = SplitTabs(line);
			SERVER_VEHICLE_SKILL skill{};
			LostArk::Shared::VEHICLE_ID ownerId = LostArk::Shared::INVALID_VEHICLE_ID;
			std::uint32_t sampleCount = 0u;
			if (8u != skillFields.size() || "VEHICLESKILL" != skillFields[0] ||
				!ParseNumber(skillFields[1], ownerId) || ownerId != vehicle.iVehicleId ||
				!ParseNumber(skillFields[2], skill.iSkillId) ||
				LostArk::Shared::INVALID_SKILL_ID == skill.iSkillId ||
				!ParseSlot(skillFields[3], skill.eSlot) ||
				!ParseNumber(skillFields[4], skill.iCooldownMs) || skill.iCooldownMs > 600000u ||
				!ParseNumber(skillFields[5], skill.iActionDurationMs) ||
				0u == skill.iActionDurationMs || skill.iActionDurationMs > 60000u ||
				!ParseNumber(skillFields[6], sampleCount) ||
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
		if (!staged.emplace(vehicle.iVehicleId, std::move(vehicle)).second)
		{
			m_strStatus = "Duplicate vehicle ID";
			return false;
		}
	}

	if (std::getline(input, line))
	{
		m_strStatus = "Vehicle bootstrap has trailing rows";
		return false;
	}

	m_Vehicles = std::move(staged);
	m_strStatus = "Loaded vehicle bootstrap";
	return true;
}

const LostArk::Server::SERVER_VEHICLE_DEFINITION*
LostArk::Server::CVehicleCatalog::Find_Vehicle(
	const LostArk::Shared::VEHICLE_ID vehicleId) const
{
	const auto iter = m_Vehicles.find(vehicleId);
	return m_Vehicles.end() == iter ? nullptr : &iter->second;
}
