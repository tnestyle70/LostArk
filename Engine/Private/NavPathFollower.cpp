// 소유한 경로와 진행 index를 갱신하는 구현이다.
#include "NavPathFollower.h"

// 요청/추적 시간과 경로 통계를 전달할 공용 profiler에 접근한다.
#include "GameInstance.h"
// 경로 검색, revision, 현재 지면 높이 샘플을 제공하는 Navigation을 호출한다.
#include "Navigation.h"
// CPU 구간 측정과 요청/확장 노드/waypoint 통계 항목을 사용한다.
#include "Profiler.h"
// 소유 객체의 위치와 수평 바라보기 방향을 변경한다.
#include "Transform.h"

// steady_clock으로 경로 질의 시간을 마이크로초 단위로 측정한다.
#include <chrono>

using namespace Engine;

// Character/Valtan의 이동 요청이 호출한다. 시작 시 기존 경로를 폐기하고 성공한 새 waypoint만 보관한다.
// 별도의 follower로 stage하는 호출자는 실패 시 자신의 기존 follower를 유지할 수 있다.
PATH_RESULT_CODE CNavPathFollower::Request_Path(
	const shared_ptr<CNavigation>& pNavigation,
	fvector_t vStartPosition,
	fvector_t vGoalPosition,
	f32_t fMaxStepHeight,
	uint32_t iMaxExpandedNodes)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Navigation.Request");
	// 새 요청은 기존 경로를 중단한다. nullptr 입력도 이 초기화 뒤 INVALID_GRID로 반환한다.
	m_pNavigation.reset();
	m_Waypoints.clear();
	m_iNextWaypoint = 0;
	m_iPathRevision = 0;
	m_iLastExpandedNodes = 0;
	if (nullptr == pNavigation)
	{
		m_eLastResult = PATH_RESULT_CODE::INVALID_GRID;
		return m_eLastResult;
	}

	// 실제 검색 결과를 임시 배열로 받아 실패한 검색의 중간 결과를 follower 경로로 사용하지 않는다.
	vector<float3_t> stagedWaypoints;
	const auto QueryBegin = std::chrono::steady_clock::now();
	m_eLastResult = pNavigation->Find_Path(
		vStartPosition,
		vGoalPosition,
		fMaxStepHeight,
		stagedWaypoints,
		iMaxExpandedNodes,
		&m_iLastExpandedNodes);
	const auto QueryEnd = std::chrono::steady_clock::now();
	const uint64_t iQueryMicroseconds = static_cast<uint64_t>(
		std::chrono::duration_cast<std::chrono::microseconds>(
			QueryEnd - QueryBegin).count());

	// 성공 여부와 관계없이 검색 횟수·확장량·시간을 기록하고 waypoint 수는 성공할 때만 누적한다.
	if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(EProfilerCounter::NavigationQueries);
		pProfiler->Add_Counter(
			EProfilerCounter::NavigationExpandedNodes,
			m_iLastExpandedNodes);
		pProfiler->Add_Counter(
			EProfilerCounter::NavigationQueryMicroseconds,
			iQueryMicroseconds);
		if (PATH_RESULT_CODE::SUCCESS == m_eLastResult)
		{
			pProfiler->Add_Counter(
				EProfilerCounter::NavigationPathCells,
				stagedWaypoints.size());
		}
	}

	if (PATH_RESULT_CODE::SUCCESS != m_eLastResult)
		return m_eLastResult;

	// 여러 점이면 시작점 0을 건너뛰고, 한 점이면 그 목표부터 이동한다. revision을 함께 저장한다.
	m_Waypoints = move(stagedWaypoints);
	m_iNextWaypoint = m_Waypoints.size() > 1 ? 1 : 0;
	m_pNavigation = pNavigation;
	m_iPathRevision = pNavigation->Get_NavigationRevision();
	return m_eLastResult;
}

// 소유 객체의 프레임 갱신이 호출한다. Navigation 소멸/변경 또는 지면 샘플 실패는 경로를 취소한다.
// false는 도착과 취소 모두에서 나오며, 앞서 적용한 위치/방향을 프레임 시작 상태로 되돌리지는 않는다.
bool_t CNavPathFollower::Update(
	const shared_ptr<CTransform>& pTransform,
	f32_t fMoveSpeed,
	f32_t fTimeDelta)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Navigation.Follow");
	// 이번 Update 동안 Navigation 수명을 확보한다. 경로가 있으면 요청 당시 revision도 대조한다.
	const shared_ptr<CNavigation> pNavigation = m_pNavigation.lock();
	if (false == m_Waypoints.empty() &&
		(nullptr == pNavigation ||
			pNavigation->Get_NavigationRevision() != m_iPathRevision))
	{
		Cancel();
		return false;
	}

	// 이동할 수 없는 입력이면 진행을 멈추되 경로는 유지한다. 이 반환은 실제 이동 성공 여부가 아니다.
	if (nullptr == pTransform ||
		false == Has_Path() ||
		fMoveSpeed <= 0.f ||
		fTimeDelta <= 0.f)
		return Has_Path();

	// 위치 후보의 XZ에 해당하는 실제 지면 Y를 구한 뒤 Transform에 적용한다.
	// 샘플 실패 시 경로만 취소하며, 직전에 LookAt이 바꾼 방향이나 이전 구간의 위치는 복원하지 않는다.
	const auto SetGroundedPosition = [&](fvector_t vPosition)
		{
			float3_t grounded{};
			if (!pNavigation->Try_SampleWalkablePoint(vPosition, grounded))
			{
				Cancel();
				return false;
			}
			pTransform->Set_State(STATE::POSITION,
				XMVectorSetW(XMLoadFloat3(&grounded), 1.f));
			return true;
		};
	// 속도는 수평 거리 기준이다. 멀리 있는 보정 waypoint의 높이로 미리 올라가지 않도록
	// 이동한 XZ가 속한 셀의 현재 지면 높이를 매번 구한다.
	f32_t fRemainingDistance = fMoveSpeed * fTimeDelta;
	while (0.f < fRemainingDistance && Has_Path())
	{
		vector_t vPosition = pTransform->Get_State(STATE::POSITION);
		vector_t vWaypoint = XMVectorSetW(
			XMLoadFloat3(&m_Waypoints[m_iNextWaypoint]),
			1.f);
		vector_t vToWaypoint = XMVectorSetY(vWaypoint - vPosition, 0.f);
		const f32_t fDistance = XMVectorGetX(
			XMVector3Length(vToWaypoint));

		// 짧은 마지막 구간도 이동 거리를 소모해야 모서리마다 속도가 더해지거나
		// 실제 표시 위치가 목표에 닿기 전에 도착으로 처리되는 일을 막을 수 있다.
		vector_t vHorizontalTarget = XMVectorSetY(
			vWaypoint,
			XMVectorGetY(vPosition));
		if (XMVectorGetX(XMVector3LengthSq(
			vHorizontalTarget - vPosition)) > 0.000001f)
			pTransform->LookAt(vHorizontalTarget);

		// 남은 거리로 이 점에 도달하면 그 구간 길이를 빼고 같은 프레임에 다음 점으로 진행한다.
		if (fDistance <= fRemainingDistance)
		{
			if (!SetGroundedPosition(vWaypoint))
				return false;
			fRemainingDistance -= fDistance;
			++m_iNextWaypoint;
			continue;
		}

		// 목표에 아직 못 미치면 남은 수평 거리만 이동한 뒤 그 지점의 지면 높이를 적용한다.
		vPosition += XMVector3Normalize(vToWaypoint) * fRemainingDistance;
		if (!SetGroundedPosition(vPosition))
			return false;
		fRemainingDistance = 0.f;
	}

	return Has_Path();
}

// 경로 진행과 차용 관계만 초기화한다. 마지막 요청의 결과·탐색량은 그대로 남겨 진단에 쓴다.
void CNavPathFollower::Cancel()
{
	m_Waypoints.clear();
	m_iNextWaypoint = 0;
	m_pNavigation.reset();
	m_iPathRevision = 0;
}

// 배열의 진행 index만 확인한다. 마지막 waypoint를 소모하면 배열을 지우지 않아도 false다.
bool_t CNavPathFollower::Has_Path() const
{
	return m_iNextWaypoint < m_Waypoints.size();
}
