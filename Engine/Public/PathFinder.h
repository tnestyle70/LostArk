#pragma once

#include "NavGrid.h"

NS_BEGIN(Engine)

enum class PATH_RESULT_CODE : uint8_t
{
	SUCCESS,
	INVALID_GRID,
	INVALID_QUERY,
	START_NOT_WALKABLE,
	GOAL_NOT_WALKABLE,
	UNREACHABLE,
	EXPANSION_LIMIT,
};

typedef struct tagPathQuery
{
	uint32_t	iStartIndex = {};
	uint32_t	iGoalIndex = {};
	f32_t		fMaxStepHeight = { 0.6f };
	uint32_t	iMaxExpandedNodes = { 16384 };
} PATH_QUERY;

typedef struct tagPathResult
{
	PATH_RESULT_CODE	eCode = { PATH_RESULT_CODE::INVALID_GRID };
	vector<uint32_t>	CellIndices;
	uint32_t		iExpandedNodes = {};
} PATH_RESULT;

class ENGINE_DLL CPathFinder final
{
private:
	typedef struct tagNodeState
	{
		uint32_t	iGeneration = {};
		uint32_t	iBestCost = {};
		uint32_t	iParentIndex = {};
		bool_t		isClosed = { false };
	} NODE_STATE;

	typedef struct tagOpenNode
	{
		uint32_t	iCellIndex = {};
		uint32_t	iCostFromStart = {};
		uint32_t	iHeuristic = {};
	} OPEN_NODE;

	struct OPEN_NODE_COMPARE final
	{
		bool operator()(
			const OPEN_NODE& Left,
			const OPEN_NODE& Right) const;
	};

public:
	CPathFinder() = default;
	~CPathFinder() = default;

	CPathFinder(const CPathFinder&) = delete;
	CPathFinder& operator=(const CPathFinder&) = delete;

public:
	PATH_RESULT Find_Path(
		const CNavGrid& NavGrid,
		const PATH_QUERY& Query);

private:
	bool_t Is_ValidGrid(const CNavGrid& NavGrid) const;
	uint32_t Compute_Heuristic(
		uint32_t iFromIndex,
		uint32_t iGoalIndex,
		uint32_t iWidth) const;
	bool_t Can_Step(
		const CNavGrid& NavGrid,
		uint32_t iFromIndex,
		int32_t iOffsetX,
		int32_t iOffsetZ,
		f32_t fMaxStepHeight,
		uint32_t& iOutToIndex) const;

	void Begin_Query(uint32_t iNumCells);
	NODE_STATE& Prepare_Node(uint32_t iCellIndex);
	void Push_Open(const OPEN_NODE& Node);
	OPEN_NODE Pop_Open();
	bool_t Build_Result(
		uint32_t iStartIndex,
		uint32_t iGoalIndex,
		PATH_RESULT& OutResult) const;

private:
	vector<NODE_STATE>	m_Nodes;
	vector<OPEN_NODE>	m_OpenNodes;
	uint32_t			m_iQueryGeneration = {};
};

NS_END
