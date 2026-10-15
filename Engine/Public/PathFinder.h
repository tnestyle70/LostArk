#pragma once

// 탐색할 격자 설명, index/높이/차단 질의와 공통 엔진 타입을 사용한다.
#include "NavGrid.h"

NS_BEGIN(Engine)

// Navigation과 경로 follower가 입력 오류·통과 불가·탐색 예산 초과를 구분한다.
// EXPANSION_LIMIT는 경로가 없다는 증명이 아니라 이번 확장 한도를 사용했다는 뜻이다.
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

// 같은 NavGrid 안의 시작/목표 index다. 단차는 월드 거리 단위, 확장 예산은 A*에서 닫는 노드 수다.
typedef struct tagPathQuery
{
	uint32_t	iStartIndex = {};
	uint32_t	iGoalIndex = {};
	f32_t		fMaxStepHeight = { 0.6f };
	uint32_t	iMaxExpandedNodes = { 16384 };
} PATH_QUERY;

// 성공 경로는 시작부터 목표까지 양 끝을 포함한 셀 index 배열이다. 월드 waypoint 변환/보정은 Navigation 책임이다.
// 확장 수는 본 A*만 센다. 동일 셀 성공 또는 사전 단절 확인은 0일 수 있다.
typedef struct tagPathResult
{
	PATH_RESULT_CODE	eCode = { PATH_RESULT_CODE::INVALID_GRID };
	vector<uint32_t>	CellIndices;
	uint32_t		iExpandedNodes = {};
} PATH_RESULT;

// Navigation 인스턴스가 소유하는 재사용 A* 작업 공간이다. 격자는 차용하고 탐색 상태/열린 노드 heap만 소유한다.
// 같은 인스턴스의 동시 질의는 허용하지 않는다. Navigation clone은 grid를 공유해도 PathFinder를 따로 만든다.
class ENGINE_DLL CPathFinder final
{
private:
	// 셀별 최선 비용·부모·닫힘 상태다. generation이 다르면 이번 질의에서 아직 준비하지 않은 값이다.
	typedef struct tagNodeState
	{
		uint32_t	iGeneration = {};
		uint32_t	iBestCost = {};
		uint32_t	iParentIndex = {};
		bool_t		isClosed = { false };
	} NODE_STATE;

	// heap에 넣은 당시 비용이다. 같은 셀의 개선 후보를 중복 삽입하고 꺼낼 때 오래된 비용을 버린다.
	typedef struct tagOpenNode
	{
		uint32_t	iCellIndex = {};
		uint32_t	iCostFromStart = {};
		uint32_t	iHeuristic = {};
	} OPEN_NODE;

	// g+h가 작은 후보를 먼저 꺼내고, 동률이면 h와 셀 index 순서로 탐색 순서를 결정한다.
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
	// Navigation이 검증된 grid와 query를 전달한다. 셀 경로와 결과 코드를 값으로 반환하고 grid는 변경하지 않는다.
	// 작업 공간은 질의마다 갱신되며 할당 예외를 PATH_RESULT_CODE로 변환하지 않는다.
	PATH_RESULT Find_Path(
		const CNavGrid& NavGrid,
		const PATH_QUERY& Query);

private:
	// 격자 크기/원점/셀 크기와 실제 셀 수를 검사한다. 개별 보행/높이 조건은 이동 검사에서 소비한다.
	bool_t Is_ValidGrid(const CNavGrid& NavGrid) const;
	// 8방향 격자의 낙관적 잔여 비용이다. 직선 10/대각선 14의 정수 비용이며 월드 거리나 이동 시간이 아니다.
	uint32_t Compute_Heuristic(
		uint32_t iFromIndex,
		uint32_t iGoalIndex,
		uint32_t iWidth) const;
	// 인접 8방향의 보행·높이 차와 대각선 양옆 경로를 확인한다. true일 때만 도착 index를 출력한다.
	bool_t Can_Step(
		const CNavGrid& NavGrid,
		uint32_t iFromIndex,
		int32_t iOffsetX,
		int32_t iOffsetZ,
		f32_t fMaxStepHeight,
		uint32_t& iOutToIndex) const;

	// 목표로 들어오는 연결을 최대 128셀 저장 범위에서 조사한다. true만 시작점 단절 증명이고 false는 도달 또는 판정 유보다.
	bool_t Is_GoalDisconnectedWithinBudget(
		const CNavGrid& NavGrid, const PATH_QUERY& Query);
	// 작업 공간 크기를 맞추고 새 generation/빈 heap을 시작한다. generation 순환 시 셀 상태 전체를 초기화한다.
	void Begin_Query(uint32_t iNumCells);
	// 이번 generation에서 처음 만난 유효 셀만 초기화한다. 반환 참조는 다음 작업 공간 재할당 전까지 유효하다.
	NODE_STATE& Prepare_Node(uint32_t iCellIndex);
	// 후보를 heap에 추가한다. 같은 셀의 이전 후보를 제거하거나 수정하지 않는다.
	void Push_Open(const OPEN_NODE& Node);
	// 비어 있지 않은 heap의 우선 후보를 제거해 값으로 반환한다. 빈 heap 검사는 호출자가 수행한다.
	OPEN_NODE Pop_Open();
	// 목표의 부모 chain을 따라간 뒤 시작→목표 순서로 뒤집는다. false의 부분 배열은 Find_Path가 비운다.
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
