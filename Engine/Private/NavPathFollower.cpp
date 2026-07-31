#include "NavPathFollower.h"

#include "GameInstance.h"
#include "Navigation.h"
#include "Profiler.h"
#include "Transform.h"

#include <chrono>

using namespace Engine;

PATH_RESULT_CODE CNavPathFollower::Request_Path(
	const shared_ptr<CNavigation>& pNavigation,
	fvector_t vStartPosition,
	fvector_t vGoalPosition,
	f32_t fMaxStepHeight,
	uint32_t iMaxExpandedNodes)
{
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

	m_Waypoints = move(stagedWaypoints);
	m_iNextWaypoint = m_Waypoints.size() > 1 ? 1 : 0;
	m_pNavigation = pNavigation;
	m_iPathRevision = pNavigation->Get_NavigationRevision();
	return m_eLastResult;
}

bool_t CNavPathFollower::Update(
	const shared_ptr<CTransform>& pTransform,
	f32_t fMoveSpeed,
	f32_t fTimeDelta)
{
	const shared_ptr<CNavigation> pNavigation = m_pNavigation.lock();
	if (false == m_Waypoints.empty() &&
		(nullptr == pNavigation ||
			pNavigation->Get_NavigationRevision() != m_iPathRevision))
	{
		Cancel();
		return false;
	}

	if (nullptr == pTransform ||
		false == Has_Path() ||
		fMoveSpeed <= 0.f ||
		fTimeDelta <= 0.f)
		return Has_Path();

	f32_t fRemainingDistance = fMoveSpeed * fTimeDelta;
	while (0.f < fRemainingDistance && Has_Path())
	{
		vector_t vPosition = pTransform->Get_State(STATE::POSITION);
		vector_t vWaypoint = XMVectorSetW(
			XMLoadFloat3(&m_Waypoints[m_iNextWaypoint]),
			1.f);
		vector_t vToWaypoint = vWaypoint - vPosition;
		const f32_t fDistance = XMVectorGetX(
			XMVector3Length(vToWaypoint));

		if (fDistance <= 0.02f)
		{
			pTransform->Set_State(STATE::POSITION, vWaypoint);
			++m_iNextWaypoint;
			continue;
		}

		vector_t vHorizontalTarget = XMVectorSetY(
			vWaypoint,
			XMVectorGetY(vPosition));
		if (XMVectorGetX(XMVector3LengthSq(
			vHorizontalTarget - vPosition)) > 0.000001f)
			pTransform->LookAt(vHorizontalTarget);

		if (fDistance <= fRemainingDistance)
		{
			pTransform->Set_State(STATE::POSITION, vWaypoint);
			fRemainingDistance -= fDistance;
			++m_iNextWaypoint;
			continue;
		}

		vPosition += XMVector3Normalize(vToWaypoint) * fRemainingDistance;
		pTransform->Set_State(
			STATE::POSITION,
			XMVectorSetW(vPosition, 1.f));
		fRemainingDistance = 0.f;
	}

	return Has_Path();
}

void CNavPathFollower::Cancel()
{
	m_Waypoints.clear();
	m_iNextWaypoint = 0;
	m_pNavigation.reset();
	m_iPathRevision = 0;
}

bool_t CNavPathFollower::Has_Path() const
{
	return m_iNextWaypoint < m_Waypoints.size();
}
