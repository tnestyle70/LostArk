#include "PathFinder.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace Engine;

namespace
{
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

PATH_RESULT CPathFinder::Find_Path(
	const CNavGrid& NavGrid,
	const PATH_QUERY& Query)
{
	PATH_RESULT Result{};

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

	if (Query.iStartIndex == Query.iGoalIndex)
	{
		Result.eCode = PATH_RESULT_CODE::SUCCESS;
		Result.CellIndices.push_back(Query.iStartIndex);
		return Result;
	}

	Begin_Query(iNumCells);

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

void CPathFinder::Push_Open(const OPEN_NODE& Node)
{
	m_OpenNodes.push_back(Node);
	std::push_heap(
		m_OpenNodes.begin(),
		m_OpenNodes.end(),
		OPEN_NODE_COMPARE{});
}

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

bool_t CPathFinder::Build_Result(
	uint32_t iStartIndex,
	uint32_t iGoalIndex,
	PATH_RESULT& OutResult) const
{
	OutResult.CellIndices.clear();
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
