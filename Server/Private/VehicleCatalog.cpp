#include "VehicleCatalog.h"

#include <Windows.h>

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
		!ParseNumber(header[1], version) || 1u != version ||
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
		if (3u != fields.size() || "VEHICLE" != fields[0] ||
			!ParseNumber(fields[1], vehicle.iVehicleId) ||
			LostArk::Shared::INVALID_VEHICLE_ID == vehicle.iVehicleId ||
			!ParseNumber(fields[2], vehicle.fMoveSpeed) ||
			!std::isfinite(vehicle.fMoveSpeed) ||
			vehicle.fMoveSpeed <= 0.f || vehicle.fMoveSpeed > 30.f)
		{
			m_strStatus = "Vehicle bootstrap row is invalid";
			return false;
		}
		if (!staged.emplace(vehicle.iVehicleId, vehicle).second)
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
