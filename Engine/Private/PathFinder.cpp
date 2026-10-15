// 질의/결과와 재사용 노드·heap 상태를 구현한다.
#include "PathFinder.h"
// 본 A*와 목표 단절 사전 검사를 별도 CPU 측정 구간으로 기록한다.
#include "Profiler.h"
// 측정 구간이 사용하는 공용 profiler를 얻는다.
#include "GameInstance.h"

// heap 유지, 부모 경로 뒤집기와 상태 초기화·최소/최대 비용 계산을 사용한다.
#include <algorithm>
// 목표 단절 사전 검사에서 최대 128셀을 고정 배열에 저장한다.
#include <array>
// 입력 유한성과 좌표/단차의 절댓값을 검사한다.
#include <cmath>
// 미방문 비용/부모 index의 sentinel과 비용 덧셈 상한을 사용한다.
#include <limits>

using namespace Engine;

namespace
{
	// 미방문/부모 없음은 uint32 최댓값으로 구분한다. 이동 비용은 미터가 아닌 탐색용 정수 가중치다.
	constexpr uint32_t INVALID_CELL =
		(numeric_limits<uint32_t>::max)();
	constexpr uint32_t INVALID_COST =
		(numeric_limits<uint32_t>::max)();
	constexpr uint32_t STRAIGHT_COST = 10;
	constexpr uint32_t DIAGONAL_COST = 14;

	typedef struct tagDirection
	{
		int32_t		iOffsetX = {};
		int32_t		iOffsetZ = {};
		uint32_t	iMoveCost = {};
	} DIRECTION;

	// 고정된 인접 방향 순서다. 직선 네 방향 뒤 대각선 네 방향을 같은 순서로 조사한다.
	constexpr DIRECTION DIRECTIONS[8] =
	{
		{  1,  0, STRAIGHT_COST },
		{ -1,  0, STRAIGHT_COST },
		{  0,  1, STRAIGHT_COST },
		{  0, -1, STRAIGHT_COST },
		{  1,  1, DIAGONAL_COST },
		{  1, -1, DIAGONAL_COST },
		{ -1,  1, DIAGONAL_COST },
		{ -1, -1, DIAGONAL_COST },
	};
}

// 표준 heap 비교를 반대로 정의해 g+h가 가장 작은 노드가 먼저 나오게 한다.
// 합계 비교는 64비트로 수행하고 같은 값은 h, 셀 index 순서로 결정한다.
bool CPathFinder::OPEN_NODE_COMPARE::operator()(
	const OPEN_NODE& Left,
	const OPEN_NODE& Right) const
{
	const uint64_t iLeftTotalCost =
		static_cast<uint64_t>(Left.iCostFromStart) + Left.iHeuristic;
	const uint64_t iRightTotalCost =
		static_cast<uint64_t>(Right.iCostFromStart) + Right.iHeuristic;

	if (iLeftTotalCost != iRightTotalCost)
		return iLeftTotalCost > iRightTotalCost;
	if (Left.iHeuristic != Right.iHeuristic)
		return Left.iHeuristic > Right.iHeuristic;
	return Left.iCellIndex > Right.iCellIndex;
}

// Navigation이 시작/목표 셀의 경로를 요청한다. 결과 배열은 지역 값이며 실패 시 사용할 경로를 반환하지 않는다.
// 격자는 읽기만 하고 재사용 작업 공간은 변경한다. 동시 질의나 탐색 도중 grid 변경은 호출자가 막아야 한다.
PATH_RESULT CPathFinder::Find_Path(
	const CNavGrid& NavGrid,
	const PATH_QUERY& Query)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Navigation.AStar");
	PATH_RESULT Result{};

	// 격자·query 범위·단차/예산과 시작/목표 보행 여부를 먼저 검증해 잘못된 index로 작업 공간에 접근하지 않는다.
	if (false == Is_ValidGrid(NavGrid))
	{
		Result.eCode = PATH_RESULT_CODE::INVALID_GRID;
		return Result;
	}

	const uint32_t iNumCells = NavGrid.Get_NumCells();
	if (Query.iStartIndex >= iNumCells ||
		Query.iGoalIndex >= iNumCells ||
		false == std::isfinite(Query.fMaxStepHeight) ||
		Query.fMaxStepHeight < 0.f ||
		0 == Query.iMaxExpandedNodes)
	{
		Result.eCode = PATH_RESULT_CODE::INVALID_QUERY;
		return Result;
	}

	if (false == NavGrid.Is_Walkable(Query.iStartIndex))
	{
		Result.eCode = PATH_RESULT_CODE::START_NOT_WALKABLE;
		return Result;
	}
	if (false == NavGrid.Is_Walkable(Query.iGoalIndex))
	{
		Result.eCode = PATH_RESULT_CODE::GOAL_NOT_WALKABLE;
		return Result;
	}

	// 같은 셀 안의 이동은 한 셀 경로로 즉시 성공한다. 실제 목표 XZ는 이후 Navigation이 반영한다.
	if (Query.iStartIndex == Query.iGoalIndex)
	{
		Result.eCode = PATH_RESULT_CODE::SUCCESS;
		Result.CellIndices.push_back(Query.iStartIndex);
		return Result;
	}

	Begin_Query(iNumCells);
	// 작은 목표 구역이 봉쇄됐는지 먼저 확인해 넓은 바닥 전체의 A* 탐색 한도를 쓰기 전에 거절한다.
	// 사전 검사로 확정할 수 없으면 본 A*의 원래 순서와 확장 예산을 그대로 사용한다.
	// 사전 검사 방문 수는 Result.iExpandedNodes에 더하지 않는다.
	if (Is_GoalDisconnectedWithinBudget(NavGrid, Query))
	{
		Result.eCode = PATH_RESULT_CODE::UNREACHABLE;
		return Result;
	}
	Begin_Query(iNumCells);

	// 사전 검사가 남긴 closed 상태와 분리된 generation에서 시작 비용 0과 부모 없음으로 본 A*를 시작한다.
	NODE_STATE& StartState = Prepare_Node(Query.iStartIndex);
	StartState.iBestCost = 0;
	StartState.iParentIndex = INVALID_CELL;

	const uint32_t iWidth = NavGrid.Get_Desc().iWidth;
	Push_Open(
		{
			Query.iStartIndex,
			0,
			Compute_Heuristic(
				Query.iStartIndex,
				Query.iGoalIndex,
				iWidth)
		});

	while (false == m_OpenNodes.empty())
	{
		const OPEN_NODE CurrentOpen = Pop_Open();
		if (CurrentOpen.iCellIndex >= m_Nodes.size())
			continue;

		// 이미 닫힌 노드와 이전 generation/비용 후보는 확장하지 않는다. 중복 heap 후보 때문에 확장 예산을 소모하지 않게 한다.
		NODE_STATE& CurrentState = m_Nodes[CurrentOpen.iCellIndex];
		if (CurrentState.iGeneration != m_iQueryGeneration ||
			true == CurrentState.isClosed ||
			CurrentOpen.iCostFromStart != CurrentState.iBestCost)
			continue;
		//Expand Node 검사
		if (Result.iExpandedNodes >= Query.iMaxExpandedNodes)
		{
			Result.eCode = PATH_RESULT_CODE::EXPANSION_LIMIT;
			return Result;
		}

		// 예산 검사 뒤 이 노드를 닫고 확장 수를 늘린다. 목표 노드도 이 수에 포함한다.
		CurrentState.isClosed = true;
		++Result.iExpandedNodes;

		if (CurrentOpen.iCellIndex == Query.iGoalIndex)
		{
			if (false == Build_Result(
				Query.iStartIndex,
				Query.iGoalIndex,
				Result))
			{
				Result.eCode = PATH_RESULT_CODE::UNREACHABLE;
				Result.CellIndices.clear();
				return Result;
			}

			Result.eCode = PATH_RESULT_CODE::SUCCESS;
			return Result;
		}

		for (const DIRECTION& Direction : DIRECTIONS)
		{
			uint32_t iNextIndex = {};
			if (false == Can_Step(
				NavGrid,
				CurrentOpen.iCellIndex,
				Direction.iOffsetX,
				Direction.iOffsetZ,
				Query.fMaxStepHeight,
				iNextIndex))
				continue;

			NODE_STATE& NextState = Prepare_Node(iNextIndex);
			if (true == NextState.isClosed)
				continue;
			if (CurrentState.iBestCost > INVALID_COST - Direction.iMoveCost)
				continue;

			// overflow 검사를 통과한 더 낮은 비용만 부모와 함께 저장한다. 동일 비용의 기존 부모는 유지한다.
			const uint32_t iCandidateCost =
				CurrentState.iBestCost + Direction.iMoveCost;
			if (iCandidateCost >= NextState.iBestCost)
				continue;

			NextState.iBestCost = iCandidateCost;
			NextState.iParentIndex = CurrentOpen.iCellIndex;
			Push_Open(
				{
					iNextIndex,
					iCandidateCost,
					Compute_Heuristic(
						iNextIndex,
						Query.iGoalIndex,
						iWidth)
				});
		}
	}

	Result.eCode = PATH_RESULT_CODE::UNREACHABLE;
	return Result;
}

// 폭·높이·월드 원점·셀 크기와 배열 수의 일치를 확인한다. 셀 수 곱셈은 64비트로 비교한다.
bool_t CPathFinder::Is_ValidGrid(const CNavGrid& NavGrid) const
{
	const CNavGrid::NAVGRID_DESC& Desc = NavGrid.Get_Desc();
	if (0 == Desc.iWidth ||
		0 == Desc.iHeight ||
		false == std::isfinite(Desc.fCellSize) ||
		Desc.fCellSize <= 0.f ||
		false == std::isfinite(Desc.fOriginX) ||
		false == std::isfinite(Desc.fOriginZ))
		return false;

	const uint64_t iExpectedCellCount =
		static_cast<uint64_t>(Desc.iWidth) * Desc.iHeight;
	return 0 != iExpectedCellCount &&
		iExpectedCellCount == NavGrid.Get_NumCells();
}

// X/Z 셀 차 중 작은 쪽만큼 대각선(14), 남은 차만큼 직선(10)으로 계산한다.
// 장애물과 단차를 무시한 잔여 비용이며 A* heap의 우선순위에 사용한다.
uint32_t CPathFinder::Compute_Heuristic(
	uint32_t iFromIndex,
	uint32_t iGoalIndex,
	uint32_t iWidth) const
{
	const int32_t iFromX = static_cast<int32_t>(iFromIndex % iWidth);
	const int32_t iFromZ = static_cast<int32_t>(iFromIndex / iWidth);
	const int32_t iGoalX = static_cast<int32_t>(iGoalIndex % iWidth);
	const int32_t iGoalZ = static_cast<int32_t>(iGoalIndex / iWidth);

	const uint32_t iDistanceX =
		static_cast<uint32_t>(std::abs(iGoalX - iFromX));
	const uint32_t iDistanceZ =
		static_cast<uint32_t>(std::abs(iGoalZ - iFromZ));
	const uint32_t iDiagonalCount = (std::min)(iDistanceX, iDistanceZ);
	const uint32_t iStraightCount =
		(std::max)(iDistanceX, iDistanceZ) - iDiagonalCount;

	return iDiagonalCount * DIAGONAL_COST +
		iStraightCount * STRAIGHT_COST;
}

// A*와 목표 역방향 사전 검사가 공유하는 한 셀 이동 조건이다. 도착 셀/단차를 확인한 뒤 성공 index만 출력한다.
// 출발 셀의 보행 가능 여부는 호출자가 확보한다. 이 helper는 범위와 도착/양옆 셀 조건을 확인한다.
bool_t CPathFinder::Can_Step(
	const CNavGrid& NavGrid,
	uint32_t iFromIndex,
	int32_t iOffsetX,
	int32_t iOffsetZ,
	f32_t fMaxStepHeight,
	uint32_t& iOutToIndex) const
{
	const CNavGrid::NAVGRID_DESC& Desc = NavGrid.Get_Desc();
	if (0 == Desc.iWidth || iFromIndex >= NavGrid.Get_NumCells())
		return false;
	if (iOffsetX < -1 || iOffsetX > 1 ||
		iOffsetZ < -1 || iOffsetZ > 1 ||
		(0 == iOffsetX && 0 == iOffsetZ))
		return false;

	const int32_t iFromX = static_cast<int32_t>(iFromIndex % Desc.iWidth);
	const int32_t iFromZ = static_cast<int32_t>(iFromIndex / Desc.iWidth);
	const int32_t iToX = iFromX + iOffsetX;
	const int32_t iToZ = iFromZ + iOffsetZ;
	if (false == NavGrid.Is_Walkable(iToX, iToZ))
		return false;

	const uint32_t iToIndex = NavGrid.To_Index(iToX, iToZ);
	auto IsValidStep = [&](uint32_t iFrom, uint32_t iTo)
		{
			return std::fabs(
				NavGrid.Get_Height(iTo) - NavGrid.Get_Height(iFrom)) <=
				fMaxStepHeight;
		};

	if (false == IsValidStep(iFromIndex, iToIndex))
		return false;

	// 대각선은 도착만 비어 있어도 충분하지 않다. 양옆 두 셀과 출발→옆→도착의 단차를 모두 검사한다.
	const bool_t isDiagonal = 0 != iOffsetX && 0 != iOffsetZ;
	if (true == isDiagonal)
	{
		const int32_t iSideXX = iFromX + iOffsetX;
		const int32_t iSideXZ = iFromZ;
		const int32_t iSideZX = iFromX;
		const int32_t iSideZZ = iFromZ + iOffsetZ;
		if (false == NavGrid.Is_Walkable(iSideXX, iSideXZ) ||
			false == NavGrid.Is_Walkable(iSideZX, iSideZZ))
			return false;

		const uint32_t iSideXIndex = NavGrid.To_Index(iSideXX, iSideXZ);
		const uint32_t iSideZIndex = NavGrid.To_Index(iSideZX, iSideZZ);
		if (false == IsValidStep(iFromIndex, iSideXIndex) ||
			false == IsValidStep(iFromIndex, iSideZIndex) ||
			false == IsValidStep(iSideXIndex, iToIndex) ||
			false == IsValidStep(iSideZIndex, iToIndex))
			return false;
	}

	iOutToIndex = iToIndex;
	return true;
}

// 작게 봉쇄된 목표 구역을 먼저 찾아 큰 바닥 전체의 A* 확장을 줄이려는 사전 검사다.
// 최대 128셀의 선행 연결만 저장하며 한도를 넘거나 시작점을 만나면 단절을 확정하지 않는다.
bool_t CPathFinder::Is_GoalDisconnectedWithinBudget(
	const CNavGrid& NavGrid, const PATH_QUERY& Query)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Navigation.GoalReachability");
	constexpr size_t PROBE_CELL_BUDGET = 128u;
	std::array<uint32_t, PROBE_CELL_BUDGET> pending{};
	size_t read = 0u;
	size_t count = 1u;
	pending[0] = Query.iGoalIndex;
	Prepare_Node(Query.iGoalIndex).isClosed = true;
	const auto& desc = NavGrid.Get_Desc();
	while (read < count)
	{
		const uint32_t current = pending[read++];
		const int32_t x = static_cast<int32_t>(current % desc.iWidth);
		const int32_t z = static_cast<int32_t>(current / desc.iWidth);
		for (const DIRECTION& direction : DIRECTIONS)
		{
			const int32_t fromX = x + direction.iOffsetX;
			const int32_t fromZ = z + direction.iOffsetZ;
			if (!NavGrid.Is_Walkable(fromX, fromZ))
				continue;
			const uint32_t from = NavGrid.To_Index(fromX, fromZ);
			NODE_STATE& state = Prepare_Node(from);
			if (state.isClosed)
				continue;
			uint32_t to = INVALID_CELL;
			// 이웃에서 현재 셀로 들어오는 방향으로 모서리·단차 조건을 검사한다.
			// 현재 조건이 대칭이어도 앞으로 방향 제한이 생겼을 때 단절 판정이 틀리지 않도록
			// 반대 방향 검사를 단순히 뒤집어 통과로 가정하지 않는다.
			if (!Can_Step(NavGrid, from, -direction.iOffsetX,
				-direction.iOffsetZ, Query.fMaxStepHeight, to) || to != current)
				continue;
			if (from == Query.iStartIndex)
				return false;
			if (count == pending.size())
				return false;
			state.isClosed = true;
			pending[count++] = from;
		}
	}
	// 목표에 들어올 수 있는 선행 연결을 모두 소진했는데 시작점을 못 찾았으므로 단절을 확정한다.
	return true;
}

// 이전 질의의 배열 용량을 재사용하고 generation으로 논리적으로 초기화한다.
// 배열 크기 변경 또는 generation이 0으로 순환한 경우에는 상태 전체를 비운다.
void CPathFinder::Begin_Query(uint32_t iNumCells)
{
	if (m_Nodes.size() != iNumCells)
	{
		m_Nodes.assign(iNumCells, NODE_STATE{});
		m_iQueryGeneration = 0;
	}

	++m_iQueryGeneration;
	if (0 == m_iQueryGeneration)
	{
		std::fill(m_Nodes.begin(), m_Nodes.end(), NODE_STATE{});
		m_iQueryGeneration = 1;
	}
	m_OpenNodes.clear();
}

// 유효 index의 상태를 이번 질의용으로 한 번만 준비한다. 오래된 비용/부모/닫힘 값이 재사용되지 않게 한다.
CPathFinder::NODE_STATE& CPathFinder::Prepare_Node(uint32_t iCellIndex)
{
	NODE_STATE& State = m_Nodes[iCellIndex];
	if (State.iGeneration != m_iQueryGeneration)
	{
		State.iGeneration = m_iQueryGeneration;
		State.iBestCost = INVALID_COST;
		State.iParentIndex = INVALID_CELL;
		State.isClosed = false;
	}
	return State;
}

// 후보를 복사해 heap 순서를 복구한다. 같은 셀의 이전 후보는 꺼낼 때 최선 비용과 대조해 거른다.
void CPathFinder::Push_Open(const OPEN_NODE& Node)
{
	m_OpenNodes.push_back(Node);
	std::push_heap(
		m_OpenNodes.begin(),
		m_OpenNodes.end(),
		OPEN_NODE_COMPARE{});
}

// 우선순위가 가장 높은 후보를 값으로 가져오고 heap에서 제거한다. 호출자는 먼저 비어 있지 않음을 확인한다.
CPathFinder::OPEN_NODE CPathFinder::Pop_Open()
{
	std::pop_heap(
		m_OpenNodes.begin(),
		m_OpenNodes.end(),
		OPEN_NODE_COMPARE{});
	const OPEN_NODE Node = m_OpenNodes.back();
	m_OpenNodes.pop_back();
	return Node;
}

// 닫힌 목표에서 부모 index를 거슬러 시작점까지 경로를 복원한다.
// 범위/generation/부모 누락 또는 셀 수를 넘는 chain이면 false이며 호출자가 부분 결과를 비운다.
bool_t CPathFinder::Build_Result(
	uint32_t iStartIndex,
	uint32_t iGoalIndex,
	PATH_RESULT& OutResult) const
{
	OutResult.CellIndices.clear();
	// 확장 수를 참고해 결과 저장 공간을 예약한다. 실제 부모 chain은 최대 전체 셀 수까지만 따라간다.
	OutResult.CellIndices.reserve(
		static_cast<size_t>(OutResult.iExpandedNodes) + 1);

	uint32_t iCurrentIndex = iGoalIndex;
	for (size_t iCount = 0; iCount < m_Nodes.size(); ++iCount)
	{
		if (iCurrentIndex >= m_Nodes.size())
			return false;

		const NODE_STATE& State = m_Nodes[iCurrentIndex];
		if (State.iGeneration != m_iQueryGeneration)
			return false;

		OutResult.CellIndices.push_back(iCurrentIndex);
		if (iCurrentIndex == iStartIndex)
		{
			std::reverse(
				OutResult.CellIndices.begin(),
				OutResult.CellIndices.end());
			return true;
		}
		if (INVALID_CELL == State.iParentIndex)
			return false;

		iCurrentIndex = State.iParentIndex;
	}
	return false;
}
