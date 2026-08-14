#pragma once

#include <cstdint>
#include <map>
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

	struct SERVER_NAVIGATION_CONDITION_CHANGE final
	{
		std::string strConditionId;
		bool bValue = false;
	};

	struct SERVER_NAVIGATION_CONDITION_STAGE final
	{
		std::uint64_t iBaseRevision = 0u;
		std::uint64_t iNextRevision = 0u;
		std::map<std::string, bool> ConditionValues;
		std::vector<std::uint16_t> BlockCounts;
		bool bChanged = false;
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
		bool Prepare_ConditionChanges(
			const std::vector<SERVER_NAVIGATION_CONDITION_CHANGE>& changes,
			SERVER_NAVIGATION_CONDITION_STAGE& outStage,
			std::string& outStatus) const;
		void Commit_ConditionChanges(
			SERVER_NAVIGATION_CONDITION_STAGE&& stage) noexcept;
		void Reset_RuntimeBlockers() noexcept;

		bool Has_Condition(const std::string& conditionId) const;
		bool Is_PointWalkableExact(float x, float z) const;
		std::uint64_t Get_Revision() const noexcept { return m_iRevision; }
		bool Sample_Position(
			float x,
			float z,
			SERVER_NAV_POINT& outPoint) const;
		bool Has_LineOfSight(
			float startX,
			float startZ,
			float endX,
			float endZ) const;

		bool Is_Loaded() const { return !m_Walkable.empty(); }
		float Get_CellSize() const { return m_fCellSize; }
		const std::string& Get_Status() const { return m_strStatus; }

	private:
		struct RUNTIME_BLOCKER_REGION final
		{
			std::string strRegionId;
			std::string strConditionId;
			bool bActivateWhenConditionTrue = true;
			std::vector<std::uint32_t> CellIndices;
		};

		bool Resolve_Cell(float x, float z, std::uint32_t& outIndex) const;
		SERVER_NAV_POINT Cell_ToPoint(std::uint32_t index) const;
		bool Is_CellWalkable(std::uint32_t index) const;
		bool Load_RuntimeBlockers(const std::string& areaId);
		void Rebuild_InitialRuntimeBlockers() noexcept;

	private:
		std::uint32_t m_iWidth = 0;
		std::uint32_t m_iHeight = 0;
		float m_fCellSize = 0.f;
		float m_fOriginX = 0.f;
		float m_fOriginZ = 0.f;
		std::vector<std::uint8_t> m_Walkable;
		std::vector<float> m_Heights;
		std::vector<RUNTIME_BLOCKER_REGION> m_RuntimeBlockerRegions;
		std::map<std::string, bool> m_ConditionValues;
		std::vector<std::uint16_t> m_BlockCounts;
		std::uint64_t m_iRevision = 0u;
		std::string m_strStatus;
	};
}
