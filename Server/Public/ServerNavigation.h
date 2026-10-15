#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace LostArk::Server
{
	/* 높이를 모르는 질의의 NaN 표식이다. 겹친 XZ region은 지면 유무를 우선하고 높이 hint가 없으면 동률을 manifest 순서로 고른다. */
	inline constexpr float NAVIGATION_HEIGHT_UNKNOWN =
		std::numeric_limits<float>::quiet_NaN();

	struct SERVER_NAV_POINT
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	// room 소유 임시 지지면이다. 기존 지면을 받치며 막힌 cell을 열지는 않는다.
	struct SERVER_NAVIGATION_SUPPORT_SURFACE final
	{
		std::string strOwnerKey;
		float fCenterX = 0.f;
		float fCenterZ = 0.f;
		float fRadiusM = 0.f;
		float fHeightY = 0.f;
		bool Contains_PointXZ(const float x, const float z) const noexcept
		{
			const double dx = static_cast<double>(x) - fCenterX;
			const double dz = static_cast<double>(z) - fCenterZ;
			return dx * dx + dz * dz <= static_cast<double>(fRadiusM) * fRadiusM;
		}
		bool operator==(const SERVER_NAVIGATION_SUPPORT_SURFACE&) const = default;
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
		/* 같은 준비·commit에 포함되는 cell별 지면 제거 region 수다. 장애물 개수를 담는 BlockCounts와 구분한다. */
		std::vector<std::uint16_t> VoidCounts;
		bool bChanged = false;
	};

	// Load 이후 room thread가 조회·누적하는 통계다. inclusive 시간이므로 SmoothPath와 내부 LineOfSight 시간은 겹친다.
	struct SERVER_NAVIGATION_QUERY_METRICS final
	{
		std::uint64_t iCalls = 0u;
		std::uint64_t iTotalNanoseconds = 0u;
		std::uint64_t iMaximumNanoseconds = 0u;
		std::uint64_t iExpandedNodes = 0u;
		std::uint64_t iReturnedPathPoints = 0u;
	};

	struct SERVER_NAVIGATION_PERFORMANCE_METRICS final
	{
		SERVER_NAVIGATION_QUERY_METRICS FindPath;
		SERVER_NAVIGATION_QUERY_METRICS ReachablePath;
		SERVER_NAVIGATION_QUERY_METRICS ProjectPoint;
		SERVER_NAVIGATION_QUERY_METRICS SmoothPath;
		SERVER_NAVIGATION_QUERY_METRICS TraversalStep;
		SERVER_NAVIGATION_QUERY_METRICS LineOfSight;
	};

	// 룸이 값으로 소유하는 이동 검사기로, 게시 격자·지면 높이·동적 차단 조건을 보관한다.
	// 같은 룸 스레드에서 경로·도달점·지지면을 질의하며 클라이언트 Transform을 직접 바꾸지 않는다.
	class CServerNavigation final
	{
	public:
		bool Load(const std::string& areaId);
		// 각 leaf detail grid를 한 번씩 합친다. region 선택 자체의 시간은 포함하지 않는다.
		SERVER_NAVIGATION_PERFORMANCE_METRICS Get_PerformanceMetrics() const;
		/* startY·hintY·fromY는 시작 XZ가 겹친 detail region 중 layer를 고르는 값이다. 현재 deck을 아는 호출자는 그 높이를 전달한다. */
		bool Find_Path(
			float startX,
			float startZ,
			float goalX,
			float goalZ,
			std::vector<SERVER_NAV_POINT>& outPath,
			float startY = NAVIGATION_HEIGHT_UNKNOWN) const;
		/* player·monster가 공유하는 시야선 기반 경로 단순화다. 투영한 실제 목표점이 보이면 마지막 cell 중심을 그 점으로 바꾼다. */
		void Smooth_Path(
			float startX,
			float startZ,
			float goalX,
			float goalZ,
			std::vector<SERVER_NAV_POINT>& path,
			float startY = NAVIGATION_HEIGHT_UNKNOWN) const;
		/* wander의 준비·실행이 쓰는 제한된 결정적 BFS다. 경로의 모든 cell 중심을 spawn 원 안으로 제한한다. */
		bool Find_PathToReachablePointWithinRadius(
			float startX,
			float startZ,
			float centerX,
			float centerZ,
			float radius,
			float minimumDestinationDistance,
			std::vector<SERVER_NAV_POINT>& outPath,
			float startY = NAVIGATION_HEIGHT_UNKNOWN) const;
		bool Project_Point(
			float x,
			float z,
			SERVER_NAV_POINT& outPoint,
			float hintY = NAVIGATION_HEIGHT_UNKNOWN) const;
		/* 현재 cell과 같은 deck의 지면에 투영한다. XZ만 가까운 아래층으로 낙하 부활 위치가 넘어가지 않게 한다. */
		bool Project_PointOnSameLevel(
			float x,
			float z,
			SERVER_NAV_POINT& outPoint,
			float hintY = NAVIGATION_HEIGHT_UNKNOWN) const;
		bool Prepare_ConditionChanges(
			const std::vector<SERVER_NAVIGATION_CONDITION_CHANGE>& changes,
			SERVER_NAVIGATION_CONDITION_STAGE& outStage,
			std::string& outStatus) const;
		void Commit_ConditionChanges(
			SERVER_NAVIGATION_CONDITION_STAGE&& stage) noexcept;
		void Reset_RuntimeBlockers() noexcept;

		bool Set_RuntimeSupportSurfaces(
			const std::vector<SERVER_NAVIGATION_SUPPORT_SURFACE>& surfaces,
			std::string& outStatus);
		std::size_t Get_RuntimeSupportSurfaceCount() const noexcept { return m_RuntimeSupportSurfaces.size(); }
		const std::vector<SERVER_NAVIGATION_SUPPORT_SURFACE>& Get_RuntimeSupportSurfaces() const noexcept { return m_RuntimeSupportSurfaces; }

		bool Has_Condition(const std::string& conditionId) const;
		// 현재 cell을 닫고 있는 저작 blocker region 수를 Debug 진단에 제공한다.
		std::size_t Get_ActiveBlockerRegionCount() const;
		bool Is_PointWalkableExact(
			float x,
			float z,
			float hintY = NAVIGATION_HEIGHT_UNKNOWN) const;
		/* 장애물 해제가 아닌 구멍 생성 조건을 지정한다. 등록된 ID·region이 있어야 하며 조건 true가 차단을 활성화하는 방향이어야 한다. */
		bool Set_VoidConditions(
			const std::set<std::string>& conditionIds,
			std::string& outStatus);
		bool Is_PointInVoidRegion(float x, float z) const;
		std::uint64_t Get_Revision() const noexcept { return m_iRevision; }
		// 공중 착지용 물리 지면을 조회한다. 걷기 장애물은 지면을 없애지 않지만 void cell은 거부한다.
		bool Sample_SurfacePosition(float x, float z, SERVER_NAV_POINT& outPoint) const;
		bool Sample_Position(
			float x,
			float z,
			SERVER_NAV_POINT& outPoint,
			float hintY = NAVIGATION_HEIGHT_UNKNOWN) const;
		/* 출발·도착 XZ의 실제 지면으로 한 걸음을 검증한다. 결과 Y는 도착 cell 값이며 먼 경유점의 높이로 보간하지 않는다. */
		bool Resolve_TraversalStep(
			float fromX,
			float fromZ,
			float toX,
			float toZ,
			SERVER_NAV_POINT& outPoint,
			float fromY = NAVIGATION_HEIGHT_UNKNOWN) const;
		bool Has_LineOfSight(
			float startX,
			float startZ,
			float endX,
			float endZ,
			float startY = NAVIGATION_HEIGHT_UNKNOWN) const;
		/* 저작한 deck 높이 차 제한이다. 0이면 단층 grid의 큰 높이 변화를 허용하도록 이 제한을 끈다. */
		bool Is_HeightTransitionAllowed(float fromY, float toY) const;

		bool Is_Loaded() const { return !m_Walkable.empty(); }
		/* base·detail grid 중 최소 cell 크기다. 호출자는 표본 간격·탐색 거리에 사용한다. */
		float Get_CellSize() const;
		/* 질의 시작 XZ가 속한 detail grid 안에서만 경로를 찾는다. 겹친 layer는 높이 hint로 고르며 지면 간격이 max(2m, 큰 step 제한의 2배)보다 좁으면 Load가 거부한다. grid·layer 간 이동은 저작 이동이 맡는다. */
		std::size_t Get_RegionCount() const noexcept { return m_Regions.size(); }
		/* 두 점이 같은 로드된 detail grid에 있어야 한다. base grid만 공유하는 경우는 제외한다. */
		bool Is_InSameDetailRegion(float x, float z, float otherX, float otherZ) const;
		/* base를 포함한 같은 navigation grid에 있는지 검사한다. 두 점 모두 로드된 grid 안이어야 한다. */
		bool Is_InSameNavigationGrid(float x, float z, float otherX, float otherZ) const;
		/* manifest ID로 지정한 detail region이 해당 위치·높이 질의에 선택되고 걸을 수 있을 때만 true다. */
		bool Is_PointWalkableInRegion(
			const std::string& regionId,
			float x,
			float z,
			float hintY = NAVIGATION_HEIGHT_UNKNOWN) const;
		float Get_MaximumTraversalStepHeight() const
		{
			return m_fMaximumTraversalStepHeight;
		}
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
		float Effective_Height(std::uint32_t index, float x, float z) const;
		bool Are_SupportTransitionsAllowed(float startX, float startZ, float endX, float endZ) const;

		bool Is_CellWalkable(std::uint32_t index) const;
		bool Is_CellTraversalAllowed(
			std::uint32_t fromIndex,
			std::uint32_t toIndex) const;
		bool Is_CellVoid(std::uint32_t index) const;
		bool Load_SurfaceMask(const std::string& areaId);
		bool Load_RuntimePolicy(const std::string& areaId);
		bool Load_RuntimeBlockers(const std::string& areaId);
		void Rebuild_InitialRuntimeBlockers() noexcept;
		bool Load_Regions(const std::string& areaId);
		/* 겹친 region은 해당 XZ의 걷는 지면을 우선한다. 그 안에서는 hint 높이와 가까운 곳, 동률·hint 없음은 manifest 순서다. 지면이 없는 후보는 전체 걷는 높이 band와의 거리로 비교한다. */
		const CServerNavigation* Select_Region(
			float x,
			float z,
			float hintY = NAVIGATION_HEIGHT_UNKNOWN) const;
		bool Contains_Point(float x, float z) const;
		bool Overlaps_Grid(const CServerNavigation& other) const;
		void Compute_WalkableBand();
		bool Get_WalkableHeightAt(float x, float z, float& outY) const;
		float Get_DistanceToWalkableBand(float y) const;
		bool Find_AmbiguousLayerOverlap(
			const CServerNavigation& other, std::string& outDetail) const;
		std::size_t Get_DeclaredBlockerRegionCount() const noexcept
		{
			return m_RuntimeBlockerRegions.size();
		}

	private:
		mutable SERVER_NAVIGATION_PERFORMANCE_METRICS m_PerformanceMetrics{};
		std::uint32_t m_iWidth = 0;
		std::uint32_t m_iHeight = 0;
		float m_fCellSize = 0.f;
		float m_fOriginX = 0.f;
		float m_fOriginZ = 0.f;
		float m_fMaximumTraversalStepHeight = 0.f;
		/* Load가 계산한 walkable cell의 높이 범위다. min > max이면 비어 있다. */
		float m_fWalkableMinY = (std::numeric_limits<float>::max)();
		float m_fWalkableMaxY = std::numeric_limits<float>::lowest();
		std::vector<std::uint8_t> m_Walkable;
		std::vector<std::uint8_t> m_Surface;
		std::vector<float> m_Heights;
		std::vector<SERVER_NAVIGATION_SUPPORT_SURFACE> m_RuntimeSupportSurfaces;
		std::vector<RUNTIME_BLOCKER_REGION> m_RuntimeBlockerRegions;
		std::map<std::string, bool> m_ConditionValues;
		std::vector<std::uint16_t> m_BlockCounts;
		std::set<std::string> m_VoidConditionIds;
		std::vector<std::uint16_t> m_VoidCounts;
		std::uint64_t m_iRevision = 0u;
		std::string m_strStatus;
		/* detail grid의 manifest ID이며 base grid에서는 비어 있다. */
		std::string m_strRegionId;
		/* manifest 순서로 detail grid를 값 소유한다. 각 자식 grid의 m_Regions는 비어 있다. */
		std::vector<CServerNavigation> m_Regions;
	};
}
