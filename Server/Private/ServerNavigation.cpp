#include "ServerNavigation.h"

#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <queue>

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

	template<typename T>
	bool ReadValue(std::ifstream& input, T& value)
	{
		return static_cast<bool>(input.read(
			reinterpret_cast<char*>(&value), sizeof(T)));
	}
}

bool LostArk::Server::CServerNavigation::Load(const std::string& areaId)
{
	m_iWidth = 0;
	m_iHeight = 0;
	m_fCellSize = 0.f;
	m_fOriginX = 0.f;
	m_fOriginZ = 0.f;
	m_Walkable.clear();
	m_Heights.clear();

	const std::filesystem::path path = Resolve_DataRoot() / L"Navigation" /
		std::filesystem::path(areaId + ".navgrid");
	std::ifstream input(path, std::ios::binary);
	if (!input || !ReadValue(input, m_iWidth) || !ReadValue(input, m_iHeight) ||
		!ReadValue(input, m_fCellSize) || !ReadValue(input, m_fOriginX) ||
		!ReadValue(input, m_fOriginZ))
	{
		m_strStatus = "Missing or truncated server navigation: " + path.string();
		return false;
	}

	const std::uint64_t cellCount64 =
		static_cast<std::uint64_t>(m_iWidth) * m_iHeight;
	if (0u == m_iWidth || 0u == m_iHeight || cellCount64 > 1000000u ||
		!std::isfinite(m_fCellSize) || m_fCellSize <= 0.f ||
		!std::isfinite(m_fOriginX) || !std::isfinite(m_fOriginZ))
	{
		m_strStatus = "Server navigation header is invalid";
		return false;
	}

	const std::size_t cellCount = static_cast<std::size_t>(cellCount64);
	std::vector<std::uint8_t> walkable(cellCount);
	std::vector<float> heights(cellCount);
	if (!input.read(reinterpret_cast<char*>(walkable.data()), walkable.size()) ||
		!input.read(reinterpret_cast<char*>(heights.data()),
			static_cast<std::streamsize>(heights.size() * sizeof(float))))
	{
		m_strStatus = "Server navigation payload is truncated";
		return false;
	}
	char trailing = 0;
	if (input.read(&trailing, 1))
	{
		m_strStatus = "Server navigation has trailing bytes";
		return false;
	}
	if (!std::all_of(walkable.begin(), walkable.end(), [](const std::uint8_t value)
		{ return value <= 1u; }) ||
		!std::all_of(heights.begin(), heights.end(), [](const float value)
		{ return std::isfinite(value); }))
	{
		m_strStatus = "Server navigation contains invalid cells";
		return false;
	}

	m_Walkable = std::move(walkable);
	m_Heights = std::move(heights);
	m_strStatus = "Loaded server navigation: " + std::to_string(m_iWidth) +
		"x" + std::to_string(m_iHeight);
	return true;
}

bool LostArk::Server::CServerNavigation::Resolve_Cell(
	const float x,
	const float z,
	std::uint32_t& outIndex) const
{
	if (!Is_Loaded() || !std::isfinite(x) || !std::isfinite(z))
		return false;
	const int centerX = static_cast<int>(std::floor((x - m_fOriginX) / m_fCellSize));
	const int centerZ = static_cast<int>(std::floor((z - m_fOriginZ) / m_fCellSize));
	constexpr int MAX_PROJECTION_RADIUS = 12;
	float nearestDistanceSquared = (std::numeric_limits<float>::max)();
	bool found = false;
	for (int radius = 0; radius <= MAX_PROJECTION_RADIUS; ++radius)
	{
		for (int dz = -radius; dz <= radius; ++dz)
		{
			for (int dx = -radius; dx <= radius; ++dx)
			{
				if (radius != 0 && std::abs(dx) != radius && std::abs(dz) != radius)
					continue;
				const int cellX = centerX + dx;
				const int cellZ = centerZ + dz;
				if (cellX < 0 || cellZ < 0 ||
					cellX >= static_cast<int>(m_iWidth) ||
					cellZ >= static_cast<int>(m_iHeight))
				{
					continue;
				}
				const std::uint32_t index = static_cast<std::uint32_t>(
					cellZ * static_cast<int>(m_iWidth) + cellX);
				if (0u == m_Walkable[index])
					continue;
				const SERVER_NAV_POINT point = Cell_ToPoint(index);
				const float deltaX = point.x - x;
				const float deltaZ = point.z - z;
				const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
				if (distanceSquared < nearestDistanceSquared)
				{
					nearestDistanceSquared = distanceSquared;
					outIndex = index;
					found = true;
				}
			}
		}
		if (found)
			return true;
	}
	return false;
}

LostArk::Server::SERVER_NAV_POINT
LostArk::Server::CServerNavigation::Cell_ToPoint(const std::uint32_t index) const
{
	const std::uint32_t x = index % m_iWidth;
	const std::uint32_t z = index / m_iWidth;
	return {
		m_fOriginX + (static_cast<float>(x) + 0.5f) * m_fCellSize,
		m_Heights[index],
		m_fOriginZ + (static_cast<float>(z) + 0.5f) * m_fCellSize
	};
}

bool LostArk::Server::CServerNavigation::Project_Point(
	const float x,
	const float z,
	SERVER_NAV_POINT& outPoint) const
{
	if (!Is_Loaded() || x < m_fOriginX || z < m_fOriginZ ||
		x >= m_fOriginX + static_cast<float>(m_iWidth) * m_fCellSize ||
		z >= m_fOriginZ + static_cast<float>(m_iHeight) * m_fCellSize)
	{
		return false;
	}
	std::uint32_t index = 0;
	if (!Resolve_Cell(x, z, index))
		return false;
	outPoint = Cell_ToPoint(index);
	return true;
}

bool LostArk::Server::CServerNavigation::Find_Path(
	const float startX,
	const float startZ,
	const float goalX,
	const float goalZ,
	std::vector<SERVER_NAV_POINT>& outPath) const
{
	outPath.clear();
	std::uint32_t start = 0;
	std::uint32_t goal = 0;
	if (!Resolve_Cell(startX, startZ, start) ||
		!Resolve_Cell(goalX, goalZ, goal))
	{
		return false;
	}
	if (start == goal)
	{
		outPath.push_back(Cell_ToPoint(goal));
		return true;
	}

	struct OPEN_NODE
	{
		float score = 0.f;
		std::uint32_t index = 0;
		bool operator<(const OPEN_NODE& other) const { return score > other.score; }
	};
	const std::size_t count = m_Walkable.size();
	std::vector<float> costs(count, (std::numeric_limits<float>::max)());
	std::vector<std::uint32_t> parents(count, (std::numeric_limits<std::uint32_t>::max)());
	std::vector<std::uint8_t> closed(count, 0u);
	std::priority_queue<OPEN_NODE> open;
	const auto heuristic = [this, goal](const std::uint32_t index)
	{
		const int x = static_cast<int>(index % m_iWidth);
		const int z = static_cast<int>(index / m_iWidth);
		const int goalX = static_cast<int>(goal % m_iWidth);
		const int goalZ = static_cast<int>(goal / m_iWidth);
		return std::hypot(static_cast<float>(goalX - x),
			static_cast<float>(goalZ - z));
	};
	costs[start] = 0.f;
	open.push({ heuristic(start), start });
	constexpr int DIRECTIONS[8][2] = {
		{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{1,-1},{-1,1},{1,1}
	};
	bool found = false;
	while (!open.empty())
	{
		const std::uint32_t current = open.top().index;
		open.pop();
		if (closed[current])
			continue;
		closed[current] = 1u;
		if (current == goal)
		{
			found = true;
			break;
		}
		const int currentX = static_cast<int>(current % m_iWidth);
		const int currentZ = static_cast<int>(current / m_iWidth);
		for (const auto& direction : DIRECTIONS)
		{
			const int nextX = currentX + direction[0];
			const int nextZ = currentZ + direction[1];
			if (nextX < 0 || nextZ < 0 || nextX >= static_cast<int>(m_iWidth) ||
				nextZ >= static_cast<int>(m_iHeight))
			{
				continue;
			}
			const std::uint32_t next = static_cast<std::uint32_t>(
				nextZ * static_cast<int>(m_iWidth) + nextX);
			if (0u == m_Walkable[next] || closed[next])
				continue;
			const bool diagonal = 0 != direction[0] && 0 != direction[1];
			if (diagonal)
			{
				const std::uint32_t sideA = static_cast<std::uint32_t>(
					currentZ * static_cast<int>(m_iWidth) + nextX);
				const std::uint32_t sideB = static_cast<std::uint32_t>(
					nextZ * static_cast<int>(m_iWidth) + currentX);
				if (0u == m_Walkable[sideA] || 0u == m_Walkable[sideB])
					continue;
			}
			const float candidate = costs[current] + (diagonal ? 1.41421356f : 1.f);
			if (candidate >= costs[next])
				continue;
			costs[next] = candidate;
			parents[next] = current;
			open.push({ candidate + heuristic(next), next });
		}
	}
	if (!found)
		return false;

	std::vector<std::uint32_t> reversePath;
	for (std::uint32_t current = goal; current != start; current = parents[current])
	{
		if (current >= parents.size() ||
			parents[current] == (std::numeric_limits<std::uint32_t>::max)())
		{
			return false;
		}
		reversePath.push_back(current);
	}
	std::reverse(reversePath.begin(), reversePath.end());
	outPath.reserve(reversePath.size());
	for (const std::uint32_t index : reversePath)
		outPath.push_back(Cell_ToPoint(index));
	return !outPath.empty();
}
