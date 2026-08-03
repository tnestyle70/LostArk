#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_NAV_POINT
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	class CServerNavigation final
	{
	public:
		bool Load(const std::string& areaId);
		bool Find_Path(
			float startX,
			float startZ,
			float goalX,
			float goalZ,
			std::vector<SERVER_NAV_POINT>& outPath) const;
		bool Project_Point(
			float x,
			float z,
			SERVER_NAV_POINT& outPoint) const;

		bool Is_Loaded() const { return !m_Walkable.empty(); }
		const std::string& Get_Status() const { return m_strStatus; }

	private:
		bool Resolve_Cell(float x, float z, std::uint32_t& outIndex) const;
		SERVER_NAV_POINT Cell_ToPoint(std::uint32_t index) const;

	private:
		std::uint32_t m_iWidth = 0;
		std::uint32_t m_iHeight = 0;
		float m_fCellSize = 0.f;
		float m_fOriginX = 0.f;
		float m_fOriginZ = 0.f;
		std::vector<std::uint8_t> m_Walkable;
		std::vector<float> m_Heights;
		std::string m_strStatus;
	};
}
